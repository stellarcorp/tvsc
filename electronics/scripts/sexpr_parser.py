#!/usr/bin/env python3
from __future__ import annotations

"""
sexpr_parser.py - Reusable, event-driven S-expression parser library with automatic tree construction.
"""

from typing import Any, Callable, Dict, Generator, Optional, Tuple


class Node:
    def __init__(self) -> None:
        self.type: Optional[str] = None
        self.properties: list[str] = []
        self.children: list[Node] = []


# Type definitions for event callbacks:
NodeOpenCallback = Callable[[Node, int], None]
NodeCloseCallback = Callable[[Node, int], None]


class SExprParser:
    """
    Event-driven S-expression parser.
    Fires callbacks when nodes open and close, automatically building and passing
    the completed subtree to the on_node_close callback.
    """

    def __init__(
        self,
        content: str,
        on_node_open: Optional[NodeOpenCallback] = None,
        on_node_close: Optional[NodeCloseCallback] = None,
    ):
        self.content = content
        self.on_node_open = on_node_open
        self.on_node_close = on_node_close
        self.stack: list[Node] = []

    def parse(self) -> None:
        """Parse the content string and trigger registered callbacks."""
        tokens = self._tokenize(self.content)

        for token_type, value, line_num in tokens:
            if token_type == "LPAREN":
                self.stack.append(Node())

            elif token_type == "RPAREN":
                if not self.stack:
                    continue  # Guard against malformed S-expressions

                current_frame = self.stack.pop()
                parent_frame = self.stack[-1] if self.stack else None
                if parent_frame:
                    parent_frame.children.append(current_frame)

                if self.on_node_close:
                    self.on_node_close(current_frame, line_num)

            elif token_type == "ATOM":
                if self.stack:
                    current_frame = self.stack[-1]
                    # The first atom immediately after '(' defines the node type
                    if current_frame.type is None:
                        current_frame.type = value
                        if self.on_node_open:
                            self.on_node_open(current_frame, line_num)
                    else:
                        current_frame.properties.append(value)

    def _tokenize(
        self, text: str
    ) -> Generator[Tuple[str, str, int], None, None]:
        """
        Tokenizer yielding (token_type, value, line_number).
        Handles string literals, parentheses, line comments, and S-expression properties.
        """
        i = 0
        n = len(text)
        line_num = 1

        while i < n:
            char = text[i]

            if char == "\n":
                line_num += 1
                i += 1
            elif char.isspace():
                i += 1
            elif char == ";":
                # Line comment: skip until newline
                while i < n and text[i] != "\n":
                    i += 1
            elif char == "(":
                yield ("LPAREN", "(", line_num)
                i += 1
            elif char == ")":
                yield ("RPAREN", ")", line_num)
                i += 1
            elif char == '"':
                start = i
                i += 1
                while i < n and text[i] != '"':
                    if text[i] == "\\" and i + 1 < n:
                        if text[i + 1] == "\n":
                            line_num += 1
                        i += 2  # Skip escape sequence
                    else:
                        if text[i] == "\n":
                            line_num += 1
                        i += 1
                if i < n:
                    i += 1  # Include closing quote
                yield ("ATOM", text[start:i], line_num)
            else:
                start = i
                while (
                    i < n
                    and not text[i].isspace()
                    and text[i] not in "();"
                ):
                    i += 1
                yield ("ATOM", text[start:i], line_num)
