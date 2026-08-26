#!/usr/bin/env python3
from __future__ import annotations

"""
Data type representing nodes in KiCad S-expressions.
"""

from typing import Callable, Optional


class Node:
    def __init__(self) -> None:
        self.type: Optional[str] = None
        self.properties: list[str] = []
        self.children: list[Node] = []

    def find_child(self, fn: Callable[[Node], bool], recurse: bool = False) -> Optional[Node]:
        nodes = self.children.copy()
        for node in nodes:
            if fn(node):
                return node
            if recurse:
                nodes.append(node.children)

        return None

    def find_children(self, fn: Callable[[Node], bool], recurse: bool = False) -> list[Node]:
        result = []
        nodes = self.children.copy()
        for node in nodes:
            if fn(node):
                result.append(node)
            if recurse:
                nodes.append(node.children)

        return result

    def __repr__(self, level: int = 0) -> str:
        indent = "\t" * level
        result = f"{indent}{self.type}: {self.properties} [\n"
        for child in self.children:
            result += child.__repr__(level + 1)
        result += f"{indent}]\n"
        return result
