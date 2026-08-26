#!/usr/bin/env pytest

import pytest
from node import Node
from sexpr_parser import SExprParser


def parse_and_collect(content: str) -> list[Node]:
    """Utility helper to parse S-expressions and return closed Node instances."""
    closed_nodes: list[Node] = []

    def on_close(node: Node, line_num: int) -> None:
        closed_nodes.append(node)

    parser = SExprParser(content, on_node_close=on_close)
    parser.parse()
    return closed_nodes


def test_find_child():
    file_path = "testdata/single_component/single_component.kicad_sch"
    nodes = parse_and_collect(file_path)

    # Ensure the parser successfully opened the file, processed contents, and closed nodes
    assert len(nodes) > 0

    # Root node is the last node to close in the bottom-up event-driven parser sequence
    root_node = nodes[-1]
    uuid_node = root_node.find_child(lambda node: node.type == "uuid")
    print(f"uuid_node: {uuid_node}")
    assert uuid_node and uuid_node.type == "uuid"


def test_find_child_recursive():
    file_path = "testdata/single_component/single_component.kicad_sch"
    nodes = parse_and_collect(file_path)

    # Ensure the parser successfully opened the file, processed contents, and closed nodes
    assert len(nodes) > 0

    # Root node is the last node to close in the bottom-up event-driven parser sequence
    root_node = nodes[-1]
    symbol_node = root_node.find_child(lambda node: node.type == "symbol", recurse=True)
    print(f"symbol_node: {symbol_node}")
    assert symbol_node and symbol_node.type == "symbol"
    lib_id = symbol_node.find_child(lambda node: node.type == "lib_id")
    assert lib_id.properties == ["power:GND"]


def test_find_children():
    file_path = "testdata/single_component/single_component.kicad_sch"
    nodes = parse_and_collect(file_path)

    # Ensure the parser successfully opened the file, processed contents, and closed nodes
    assert len(nodes) > 0

    # Root node is the last node to close in the bottom-up event-driven parser sequence
    root_node = nodes[-1]
    symbols = root_node.find_children(lambda node: node.type == "symbol")
    print(f"symbols: {symbols}")
    for node in symbols:
        assert node and node.type == "symbol"


if __name__ == "__main__":
    import sys
    import pytest
    sys.exit(pytest.main(sys.argv))
