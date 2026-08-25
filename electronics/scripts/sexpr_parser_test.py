#!/usr/bin/env pytest
"""
test_sexpr_parser.py - Unit tests for sexpr_parser.py using pytest.
"""

import pytest
from sexpr_parser import SExprParser


def parse_and_collect(content: str):
    """Utility helper to parse S-expressions and return closed node frames."""
    closed_nodes = []

    def on_close(frame, line_num):
        # Frame format: {"type": str, "line": int, "atoms": list}
        closed_nodes.append({
            "type": frame["type"] if frame["type"] else "UNKNOWN",
            "atoms": frame["atoms"],
            "line": frame["line"]
        })

    parser = SExprParser(content, on_node_close=on_close)
    parser.parse()
    return closed_nodes


def test_node_type_identification():
    """Verify parser correctly extracts the node type (first atom following '(')."""
    sexpr = '(footprint "Resistor_SMD:R_0805" (layer "F.Cu"))'
    nodes = parse_and_collect(sexpr)

    # Note: Event-driven close callback receives inner node first, then outer node
    child_node = nodes[0]
    parent_node = nodes[1]

    assert child_node["type"] == "layer"
    assert parent_node["type"] == "footprint"


def test_empty_node_type_identification():
    """Verify parser handles empty nodes '()' without crashing and assigns fallback type."""
    sexpr = '()'
    nodes = parse_and_collect(sexpr)

    assert len(nodes) == 1
    assert nodes[0]["type"] == "UNKNOWN"


def test_node_atoms_extraction():
    """Verify parser extracts all top-level string/number atoms belonging to a node."""
    sexpr = '(at 145.0 90.0 90)'
    nodes = parse_and_collect(sexpr)

    assert len(nodes) == 1
    assert nodes[0]["type"] == "at"
    assert nodes[0]["atoms"] == ["145.0", "90.0", "90"]


def test_node_with_only_type_and_no_atoms():
    """Verify parser handles nodes consisting solely of a type label with no extra atoms."""
    sexpr = '(mirrored)'
    nodes = parse_and_collect(sexpr)

    assert len(nodes) == 1
    assert nodes[0]["type"] == "mirrored"
    assert nodes[0]["atoms"] == []


def test_empty_node_atoms_list():
    """Verify parser returns an empty atoms list for empty parenthetical nodes '()'."""
    sexpr = '()'
    nodes = parse_and_collect(sexpr)

    assert len(nodes) == 1
    assert nodes[0]["atoms"] == []


def test_nested_child_nodes():
    """Verify node type and atom extractions across multi-level nested trees."""
    sexpr = """
    (target_sync_manifest "carrier_board"
      (block_instance "PWR_MOD_01"
        (placement
          (at 145.0 90.0)
          (rotation 90)
          ()
        )
      )
    )
    """
    nodes = parse_and_collect(sexpr)

    # Expected evaluation/closure sequence (bottom-up execution on stack popping):
    # 1. (at 145.0 90.0)
    # 2. (rotation 90)
    # 3. ()
    # 4. (placement ...)
    # 5. (block_instance "PWR_MOD_01" ...)
    # 6. (target_sync_manifest "carrier_board" ...)

    assert len(nodes) == 6

    # Depth Level 3 Nodes
    node_at = nodes[0]
    assert node_at["type"] == "at"
    assert node_at["atoms"] == ["145.0", "90.0"]

    node_rot = nodes[1]
    assert node_rot["type"] == "rotation"
    assert node_rot["atoms"] == ["90"]

    node_empty_child = nodes[2]
    assert node_empty_child["type"] == "UNKNOWN"
    assert node_empty_child["atoms"] == []

    # Depth Level 2 Node
    node_placement = nodes[3]
    assert node_placement["type"] == "placement"
    assert node_placement["atoms"] == []  # Nested parenthetical nodes are not pushed as primitive string atoms

    # Depth Level 1 Node
    node_instance = nodes[4]
    assert node_instance["type"] == "block_instance"
    assert node_instance["atoms"] == ['"PWR_MOD_01"']

    # Depth Level 0 (Root) Node
    node_root = nodes[5]
    assert node_root["type"] == "target_sync_manifest"
    assert node_root["atoms"] == ['"carrier_board"']


if __name__ == "__main__":
    import sys
    import pytest
    sys.exit(pytest.main(sys.argv))
