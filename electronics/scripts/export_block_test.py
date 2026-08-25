#!/usr/bin/env pytest

import json
import pytest
from pathlib import Path
from export_block import KiCadBlockExporter

TESTDATA_DIR = Path(__file__).parent / "testdata"


@pytest.fixture
def single_component_project():
    dir = TESTDATA_DIR / "single_component"
    sch_file = dir / "single_component.kicad_sch"
    pcb_file = dir / "single_component.kicad_pcb"
    assert sch_file.is_file(), f"schematic file '{sch_file}' does not exist"
    assert pcb_file.is_file(), f"PCB file '{pcb_file}' does not exist"
    return sch_file, pcb_file


@pytest.fixture
def trivial_project(tmp_path):
    dir = tmp_path / "trivial_project"
    dir.mkdir(exist_ok=True)
    sch_file = dir / "board.kicad_sch"
    sch_file.write_text(
        """(kicad_sch
  (version 20260000)
  (sheet
    (pin "VOUT" (uuid "3b8e1a2f-8c7d-4e5f-9a1b-2c3d4e5f6a7b"))
    (pin "GND" (uuid "11223344-5566-7788-9900-aabbccddeeff"))
  )
)""", encoding="utf-8")

    pcb_file = dir / "board.kicad_pcb"
    pcb_file.write_text(
        """(kicad_pcb
  (version 20260000)
  (footprint "Resistor_SMD:R_0805"
    (layer "F.Cu")
    (at 100.0 50.0 90)
    (property "Reference" "R101")
  )
)""", encoding="utf-8")
    return sch_file, pcb_file


@pytest.fixture
def base_project(tmp_path):
    valid_dir = tmp_path / "valid_project"
    valid_dir.mkdir(exist_ok=True)
    sch_file = valid_dir / "board.kicad_sch"
    sch_file.write_text(
        """(kicad_sch
  (version 20260000)
  (sheet
    (pin "VOUT" (uuid "3b8e1a2f-8c7d-4e5f-9a1b-2c3d4e5f6a7b"))
    (pin "GND" (uuid "11223344-5566-7788-9900-aabbccddeeff"))
  )
)""", encoding="utf-8")

    pcb_file = valid_dir / "board.kicad_pcb"
    pcb_file.write_text(
        """(kicad_pcb
  (version 20260000)
  (footprint "Resistor_SMD:R_0805"
    (layer "F.Cu")
    (at 100.0 50.0 90)
    (property "Reference" "R101")
  )
  (footprint "Capacitor_SMD:C_0603"
    (layer "B.Cu")
    (at 125.4 60.16 0)
    (property "Reference" "C105")
  )
  (segment
    (start 100.0 50.0)
    (end 125.4 60.16)
    (width 0.5)
    (layer "F.Cu")
  )
)""", encoding="utf-8")
    return sch_file, pcb_file


@pytest.fixture
def duplicate_anchor_project(tmp_path):
    dup_dir = tmp_path / "duplicate_anchor_project"
    dup_dir.mkdir(exist_ok=True)
    sch_file = dup_dir / "board.kicad_sch"
    sch_file.write_text("(kicad_sch)", encoding="utf-8")
    pcb_file = dup_dir / "board.kicad_pcb"
    pcb_file.write_text(
        """(kicad_pcb
  (footprint (at 10.0 10.0) (property "Reference" "U101"))
  (footprint (at 20.0 20.0) (property "Reference" "U101"))
)""", encoding="utf-8")
    return sch_file, pcb_file


@pytest.fixture
def malformed_parentheses_project(tmp_path):
    bad_sexpr_dir = tmp_path / "bad_sexpr_project"
    bad_sexpr_dir.mkdir(exist_ok=True)
    sch_file = bad_sexpr_dir / "board.kicad_sch"
    sch_file.write_text("(kicad_sch (pin \"VOUT\"", encoding="utf-8")
    pcb_file = bad_sexpr_dir / "board.kicad_pcb"
    pcb_file.write_text(
        """(kicad_pcb
  (footprint (at 100.0 50.0) (property "Reference" "U101")
)""", encoding="utf-8")
    return sch_file, pcb_file


@pytest.fixture
def special_characters_project(tmp_path):
    escaped_dir = tmp_path / "special_chars_project"
    escaped_dir.mkdir(exist_ok=True)
    sch_file = escaped_dir / "board.kicad_sch"
    sch_file.write_text(
        """(kicad_sch
  (pin "BUS / +5V (AUX)" (uuid "uuid-special-123"))
)""", encoding="utf-8")
    pcb_file = escaped_dir / "board.kicad_pcb"
    pcb_file.write_text(
        """(kicad_pcb
  (footprint (at -50.0 -50.0) (property "Reference" "U101"))
)""", encoding="utf-8")
    return sch_file, pcb_file


def test_can_find_anchor(trivial_project):
    """Verify that specifying a non-existent anchor raises an explicit error."""
    sch_file, pcb_file = trivial_project

    exporter = KiCadBlockExporter(
        sch_path=sch_file,
        pcb_path=pcb_file,
        anchor_refdes="R101",
        block_name="test_can_find_anchor"
    )

    exporter.extract_anchor_and_pcb_data()
    anchor_component = next(fp for fp in exporter.footprints if fp["refdes"] == "R101")
    assert anchor_component != None


def test_missing_anchor_refdes_raises_value_error(base_project):
    """Verify that specifying a non-existent anchor raises an explicit error."""
    sch_file, pcb_file = base_project

    exporter = KiCadBlockExporter(
        sch_path=sch_file,
        pcb_path=pcb_file,
        anchor_refdes="U999",  # Non-existent
        block_name="test_missing_anchor"
    )

    with pytest.raises(ValueError, match="Anchor reference designator 'U999' not found"):
        exporter.extract_anchor_and_pcb_data()


def test_anchor_at_origin(base_project):
    """Verify relative coordinates (delta X, Y) are correctly calculated from anchor U101."""
    sch_file, pcb_file = base_project

    exporter = KiCadBlockExporter(
        sch_path=sch_file,
        pcb_path=pcb_file,
        anchor_refdes="R101",
        block_name="test_offsets"
    )
    exporter.extract_anchor_and_pcb_data()

    anchor_component = next(fp for fp in exporter.footprints if fp["refdes"] == "R101")
    assert anchor_component["rel_at"] == (0., 0.)
    assert anchor_component["layer"] == "F.Cu"


def test_anchor_relative_coordinate_offsets(base_project):
    """Verify relative coordinates (delta X, Y) are correctly calculated from anchor U101."""
    sch_file, pcb_file = base_project

    exporter = KiCadBlockExporter(
        sch_path=sch_file,
        pcb_path=pcb_file,
        anchor_refdes="R101",
        block_name="test_offsets"
    )
    exporter.extract_anchor_and_pcb_data()

    c105_data = next(fp for fp in exporter.footprints if fp["refdes"] == "C105")
    assert c105_data["rel_at"] == (25.4, 10.16)
    assert c105_data["layer"] == "B.Cu"


def test_extreme_negative_coordinates(special_characters_project):
    """Verify negative and zero-crossing coordinates compute accurately without underflow."""
    sch_file, pcb_file = special_characters_project

    exporter = KiCadBlockExporter(
        sch_path=sch_file,
        pcb_path=pcb_file,
        anchor_refdes="U101",
        block_name="test_negative_coords"
    )
    exporter.extract_anchor_and_pcb_data()

    assert exporter.anchor_pos == (-50.0, -50.0)
    u101_data = next(fp for fp in exporter.footprints if fp["refdes"] == "U101")
    assert u101_data["rel_at"] == (0.0, 0.0)


def test_extract_schematic_pins_and_special_characters(special_characters_project):
    """Verify hierarchical pin names containing spaces, slashes, and symbols are extracted intact."""
    sch_file, pcb_file = special_characters_project

    exporter = KiCadBlockExporter(
        sch_path=sch_file,
        pcb_path=pcb_file,
        anchor_refdes="U101",
        block_name="test_special_pins"
    )
    exporter.extract_schematic_connection_points()

    assert len(exporter.connection_points) == 1
    pin = exporter.connection_points[0]
    assert pin["pin_name"] == "BUS / +5V (AUX)"
    assert pin["hierarchical_pin_uuid"] == "uuid-special-123"


def test_json_sidecar_generation_when_missing(tmp_path):
    """Verify that if no JSON file exists, a new sidecar file with default metadata is generated."""
    sch_file, pcb_file = base_project(tmp_path)

    exporter = KiCadBlockExporter(
        sch_path=sch_file,
        pcb_path=pcb_file,
        anchor_refdes="U101",
        block_name="power_stage"
    )
    exporter.extract_anchor_and_pcb_data()
    exporter.extract_schematic_connection_points()

    out_block, out_json = exporter.write_block_package(tmp_path)

    assert out_block.exists()
    assert out_json.exists()

    # Validate generated JSON content
    data = json.loads(out_json.read_text(encoding="utf-8"))
    assert data["block_name"] == "power_stage"
    assert data["anchor_refdes"] == "U101"
    assert data["footprints_count"] == 2
    assert data["tracks_count"] == 1


if __name__ == "__main__":
    import sys
    sys.exit(pytest.main(sys.argv))
