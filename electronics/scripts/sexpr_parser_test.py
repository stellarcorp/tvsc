#!/usr/bin/env pytest
"""
test_sexpr_parser.py - Unit tests for sexpr_parser.py using pytest.
"""

import pytest
from sexpr_parser import Node, SExprParser


def parse_and_collect(content: str) -> list[Node]:
    """Utility helper to parse S-expressions and return closed Node instances."""
    closed_nodes: list[Node] = []

    def on_close(node: Node, line_num: int) -> None:
        closed_nodes.append(node)

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

    assert child_node.type == "layer"
    assert parent_node.type == "footprint"


def test_empty_node_type_identification():
    """Verify parser handles empty nodes '()' without crashing and leaves type as None."""
    sexpr = '()'
    nodes = parse_and_collect(sexpr)

    assert len(nodes) == 1
    assert nodes[0].type is None


def test_node_atoms_extraction():
    """Verify parser extracts all top-level string/number atoms belonging to a node."""
    sexpr = '(at 145.0 90.0 90)'
    nodes = parse_and_collect(sexpr)

    assert len(nodes) == 1
    assert nodes[0].type == "at"
    assert nodes[0].properties == ["145.0", "90.0", "90"]


def test_node_with_only_type_and_no_atoms():
    """Verify parser handles nodes consisting solely of a type label with no extra atoms."""
    sexpr = '(mirrored)'
    nodes = parse_and_collect(sexpr)

    assert len(nodes) == 1
    assert nodes[0].type == "mirrored"
    assert nodes[0].properties == []


def test_empty_node_atoms_list():
    """Verify parser returns an empty properties list for empty parenthetical nodes '()'."""
    sexpr = '()'
    nodes = parse_and_collect(sexpr)

    assert len(nodes) == 1
    assert nodes[0].properties == []


def test_nested_child_nodes():
    """Verify node type and property extractions across multi-level nested trees."""
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
    assert node_at.type == "at"
    assert node_at.properties == ["145.0", "90.0"]

    node_rot = nodes[1]
    assert node_rot.type == "rotation"
    assert node_rot.properties == ["90"]

    node_empty_child = nodes[2]
    assert node_empty_child.type is None
    assert node_empty_child.properties == []

    # Depth Level 2 Node
    node_placement = nodes[3]
    assert node_placement.type == "placement"
    assert node_placement.properties == []  # Nested parenthetical nodes are attached to children, not properties

    # Depth Level 1 Node
    node_instance = nodes[4]
    assert node_instance.type == "block_instance"
    assert node_instance.properties == ['"PWR_MOD_01"']

    # Depth Level 0 (Root) Node
    node_root = nodes[5]
    assert node_root.type == "target_sync_manifest"
    assert node_root.properties == ['"carrier_board"']


if __name__ == "__main__":
    import sys
    import pytest
    sys.exit(pytest.main(sys.argv))
