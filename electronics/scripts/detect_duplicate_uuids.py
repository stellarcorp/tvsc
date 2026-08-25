#!/usr/bin/env python3
"""
detect_duplicate_uuids.py - Scans two KiCad project directories and reports
shared/duplicate UUID intersections across all project files.
"""

import sys
from collections import defaultdict
from pathlib import Path
from sexpr_parser import SExprParser


def parse_project_files(project_dir: Path) -> defaultdict:
    """
    Parses all S-expression files in a project directory into a combined UUID map.
    """
    extensions = {".kicad_sch", ".kicad_pcb", ".kicad_pro", ".kicad_wks"}
    combined_uuid_map = defaultdict(list)

    for file_path in project_dir.rglob("*"):
        if file_path.is_file() and file_path.suffix in extensions:
            try:
                content = file_path.read_text(
                    encoding="utf-8", errors="ignore"
                )
                relative_file_path = file_path.relative_to(project_dir)

                # Stateful tracker for callback context
                file_uuid_map = defaultdict(list)

                def on_node_close(
                    current_node: dict, line_num: int
                ):
                    node_type = current_node["type"]
                    if node_type == "uuid":
                        uuid_val = (
                            current_node["atoms"][0]
                            if current_node["atoms"]
                            else None
                        )
                        if not uuid_val:
                            return

                        uuid_val = uuid_val.strip('"').lower()

                        # Inspect stack on parser instance if accessible,
                        # or derive parent from node_type
                        parent_type = "file_root"
                        parent_line = current_node["line"]

                        if parser.stack:
                            parent_node = parser.stack[-1]
                            parent_type = (
                                parent_node["type"]
                                if parent_node["type"]
                                else "root"
                            )
                            parent_line = parent_node["line"]

                        file_uuid_map[uuid_val].append(
                            {
                                "file": relative_file_path,
                                "node_type": parent_type,
                                "line": parent_line,
                            }
                        )

                parser = SExprParser(content, on_node_close=on_node_close)
                parser.parse()

                for uuid_val, occurrences in file_uuid_map.items():
                    combined_uuid_map[uuid_val].extend(occurrences)

            except Exception as e:
                print(f"Error parsing {file_path}: {e}", file=sys.stderr)

    return combined_uuid_map


def generate_intersection_report(proj1_dir: Path, proj2_dir: Path):
    proj1_map = parse_project_files(proj1_dir)
    proj2_map = parse_project_files(proj2_dir)

    shared_uuids = set(proj1_map.keys()) & set(proj2_map.keys())

    # Priority ranking: Critical wrapper types rank highest to appear at bottom
    type_priority = {
        "kicad_pro": 100,
        "kicad_sch": 90,
        "kicad_pcb": 80,
        "sheet": 70,
        "symbol": 60,
        "footprint": 50,
        "pad": 40,
        "segment": 30,
        "via": 20,
        "wire": 10,
    }

    def get_uuid_priority(uuid_val):
        nodes = proj1_map[uuid_val] + proj2_map[uuid_val]
        return max(type_priority.get(node["node_type"], 0) for node in nodes)

    sorted_uuids = sorted(
        shared_uuids, key=lambda u: (get_uuid_priority(u), u)
    )

    for uuid_val in sorted_uuids:
        occurrences = []
        for occ in proj1_map[uuid_val]:
            occurrences.append(
                (proj1_dir / occ["file"], occ["line"], occ["node_type"])
            )
        for occ in proj2_map[uuid_val]:
            occurrences.append(
                (proj2_dir / occ["file"], occ["line"], occ["node_type"])
            )

        types = {occ[2] for occ in occurrences}
        show_type = len(types) > 1

        print(f"UUID: {uuid_val} {types}")
        for file_path, line_num, node_type in occurrences:
            if show_type:
                print(f"\t{file_path}:{line_num} ({node_type})")
            else:
                print(f"\t{file_path}:{line_num}")

    print(f"Total intersections: {len(shared_uuids)}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(
            "Usage: python detect_duplicate_uuids.py <path/to/project1>"
            " <path/to/project2>"
        )
        sys.exit(1)

    p1 = Path(sys.argv[1])
    p2 = Path(sys.argv[2])

    if not p1.is_dir() or not p2.is_dir():
        print(
            "Error: Both arguments must be valid directories.", file=sys.stderr
        )
        sys.exit(1)

    generate_intersection_report(p1, p2)
