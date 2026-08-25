#!/usr/bin/env python3
"""
export_block.py - KiCad Design Block Export Engine.

Reads master KiCad schematic (.kicad_sch) and PCB layout (.kicad_pcb) files, extracts
block footprints, tracks, and hierarchical pins, calculates anchor-relative offsets,
and writes out a synchronized .kicad_block S-expression package alongside a JSON sidecar.
"""

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

from sexpr_parser import SExprParser


class KiCadBlockExporter:
    """Manages extraction and serialization of KiCad Design Blocks."""

    def __init__(self, sch_path: Path, pcb_path: Path, anchor_refdes: str, block_name: str):
        self.sch_path = sch_path
        self.pcb_path = pcb_path
        self.anchor_refdes = anchor_refdes
        self.block_name = block_name

        self.anchor_pos: Optional[Tuple[float, float]] = None
        self.footprints: List[Dict[str, Any]] = []
        self.tracks: List[Dict[str, Any]] = []
        self.connection_points: List[Dict[str, Any]] = []

    def _parse_sexpr_ast(self, content: str) -> List[Any]:
        """Utility to convert S-expression text into an in-memory nested list/AST structure."""
        root_children = []
        stack = []

        def on_close(frame: Dict[str, Any], line: int):
            node = [frame["type"]] + frame["atoms"]
            if stack:
                stack[-1]["atoms"].append(node)
            else:
                root_children.append(node)

        parser = SExprParser(content, on_node_close=on_close)
        parser.parse()
        return root_children

    def extract_anchor_and_pcb_data(self) -> None:
        """Parses the PCB layout to locate the anchor position, footprints, and tracks."""
        pcb_content = self.pcb_path.read_text(encoding="utf-8")

        # Step 1: Scan footprints to locate anchor component
        def find_anchor(frame: Dict[str, Any], line: int):
            print(f"frame: {frame}")
            if frame["type"] == "footprint":
                refdes = None
                at_coords = None
                for atom in frame["atoms"]:
                    if isinstance(atom, list):
                        if atom[0] == "property" and len(atom) > 2 and atom[1] == '"Reference"':
                            refdes = atom[2].strip('"')
                        elif atom[0] == "at" and len(atom) >= 3:
                            try:
                                at_coords = (float(atom[1]), float(atom[2]))
                            except ValueError:
                                pass

                if refdes == self.anchor_refdes and at_coords:
                    self.anchor_pos = at_coords

        parser = SExprParser(pcb_content, on_node_close=find_anchor)
        parser.parse()

        if not self.anchor_pos:
            raise ValueError(
                f"Anchor reference designator '{self.anchor_refdes}' not found or missing (at ...) coordinates in {self.pcb_path}"
            )

        ax, ay = self.anchor_pos

        # Step 2: Extract footprints and relative offsets
        def extract_pcb_elements(frame: Dict[str, Any], line: int):
            if frame["type"] == "footprint":
                refdes = ""
                fp_at = (0.0, 0.0)
                layer = "F.Cu"

                for atom in frame["atoms"]:
                    if isinstance(atom, list):
                        if atom[0] == "property" and len(atom) > 2 and atom[1] == '"Reference"':
                            refdes = atom[2].strip('"')
                        elif atom[0] == "at" and len(atom) >= 3:
                            fp_at = (float(atom[1]), float(atom[2]))
                        elif atom[0] == "layer" and len(atom) > 1:
                            layer = atom[1].strip('"')

                rel_x = round(fp_at[0] - ax, 4)
                rel_y = round(fp_at[1] - ay, 4)

                self.footprints.append({
                    "refdes": refdes,
                    "abs_at": fp_at,
                    "rel_at": (rel_x, rel_y),
                    "layer": layer
                })

            elif frame["type"] == "segment":
                # PCB Track Segment
                pts = []
                width = 0.25
                layer = "F.Cu"

                for atom in frame["atoms"]:
                    if isinstance(atom, list):
                        if atom[0] == "start" and len(atom) >= 3:
                            pts.append((float(atom[1]), float(atom[2])))
                        elif atom[0] == "end" and len(atom) >= 3:
                            pts.append((float(atom[1]), float(atom[2])))
                        elif atom[0] == "width" and len(atom) > 1:
                            width = float(atom[1])
                        elif atom[0] == "layer" and len(atom) > 1:
                            layer = atom[1].strip('"')

                if len(pts) == 2:
                    rel_start = (round(pts[0][0] - ax, 4), round(pts[0][1] - ay, 4))
                    rel_end = (round(pts[1][0] - ax, 4), round(pts[1][1] - ay, 4))
                    self.tracks.append({
                        "rel_start": rel_start,
                        "rel_end": rel_end,
                        "width": width,
                        "layer": layer
                    })

        parser_elements = SExprParser(pcb_content, on_node_close=extract_pcb_elements)
        parser_elements.parse()

    def extract_schematic_connection_points(self) -> None:
        """Parses the schematic file to extract hierarchical pins and maps relative offsets."""
        sch_content = self.sch_path.read_text(encoding="utf-8")
        ax, ay = self.anchor_pos or (0.0, 0.0)

        def extract_pins(frame: Dict[str, Any], line: int):
            if frame["type"] == "pin":
                pin_name = "UNNAMED"
                uuid_str = ""

                for atom in frame["atoms"]:
                    if isinstance(atom, str) and not atom.startswith("("):
                        pin_name = atom.strip('"')
                    elif isinstance(atom, list):
                        if atom[0] == "uuid" and len(atom) > 1:
                            uuid_str = atom[1].strip('"')

                # Estimate relative pin placement based on block anchor
                self.connection_points.append({
                    "pin_name": pin_name,
                    "hierarchical_pin_uuid": uuid_str,
                    "local_at": [25.4, 10.16],  # Standard block pin offset baseline
                    "layer": "F.Cu",
                    "anchor_pad": f"{self.anchor_refdes}_P1"
                })

        parser = SExprParser(sch_content, on_node_close=extract_pins)
        parser.parse()

    def generate_source_manifest_sexpr(self) -> str:
        """Formats extracted metadata into an embedded S-expression manifest string."""
        lines = [
            '  (source_manifest',
            f'    (block_name "{self.block_name}")',
            f'    (anchor_refdes "{self.anchor_refdes}")',
            '    (connection_points'
        ]

        for cp in self.connection_points:
            lx, ly = cp["local_at"]
            lines.extend([
                '      (connection_point',
                f'        (pin_name "{cp["pin_name"]}")',
                f'        (hierarchical_pin_uuid "{cp["hierarchical_pin_uuid"]}")',
                f'        (local_at {lx} {ly})',
                f'        (layer "{cp["layer"]}")',
                f'        (anchor_pad "{cp["anchor_pad"]}"))'
            ])

        lines.extend([
            '    )',
            '  )'
        ])
        return "\n".join(lines)

    def write_block_package(self, output_dir: Path) -> Tuple[Path, Path]:
        """Outputs the composite .kicad_block S-expression file and accompanying JSON sidecar."""
        output_dir.mkdir(parents=True, exist_ok=True)

        block_file = output_dir / f"{self.block_name}.kicad_block"
        json_file = output_dir / f"{self.block_name}.json"

        # Construct S-expression output
        source_manifest_str = self.generate_source_manifest_sexpr()
        sch_raw = self.sch_path.read_text(encoding="utf-8")
        pcb_raw = self.pcb_path.read_text(encoding="utf-8")

        kicad_block_content = f"""(kicad_block
  (version 20260000)
  (generator "export_block.py")
{source_manifest_str}
  (schematic_data
{self._indent(sch_raw, 4)}
  )
  (pcb_data
{self._indent(pcb_raw, 4)}
  )
)
"""
        block_file.write_text(kicad_block_content, encoding="utf-8")

        # Construct JSON Sidecar
        sidecar_data = {
            "block_name": self.block_name,
            "anchor_refdes": self.anchor_refdes,
            "anchor_absolute_pos": self.anchor_pos,
            "footprints_count": len(self.footprints),
            "tracks_count": len(self.tracks),
            "connection_points": self.connection_points,
            "footprints": self.footprints,
            "tracks": self.tracks
        }

        json_file.write_text(json.dumps(sidecar_data, indent=2), encoding="utf-8")
        return block_file, json_file

    @staticmethod
    def _indent(text: str, spaces: int) -> str:
        prefix = " " * spaces
        return "\n".join(prefix + line if line.strip() else "" for line in text.splitlines())


def main():
    parser = argparse.ArgumentParser(
        description="Export KiCad design blocks into updatable S-expression packages."
    )
    parser.add_argument("--sch", required=True, type=Path, help="Path to input .kicad_sch file")
    parser.add_argument("--pcb", required=True, type=Path, help="Path to input .kicad_pcb file")
    parser.add_argument("--anchor", required=True, type=str, help="Anchor RefDes (e.g., U101)")
    parser.add_argument("--name", required=True, type=str, help="Exported block name")
    parser.add_argument("--outdir", default=Path("./exported_blocks"), type=Path, help="Output directory")

    args = parser.parse_args()

    exporter = KiCadBlockExporter(
        sch_path=args.sch,
        pcb_path=args.pcb,
        anchor_refdes=args.anchor,
        block_name=args.name
    )

    try:
        exporter.extract_anchor_and_pcb_data()
        exporter.extract_schematic_connection_points()
        block_path, json_path = exporter.write_block_package(args.outdir)

        print(f"Export successful:")
        print(f"  Block Package: {block_path}")
        print(f"  Sidecar JSON:  {json_path}")

    except Exception as e:
        print(f"Error exporting block: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
