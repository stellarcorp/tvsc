#!/usr/bin/env python3
"""
sexpr_parser.py - Reusable, event-driven S-expression parser library.
"""

from typing import Callable, Optional, Dict, Any, Generator, Tuple

# Type definitions for event callbacks:
# fn(node_frame: dict, line_num: int) -> None
CallbackFn = Callable[[str, Dict[str, Any], int], None]


class SExprParser:
    """
    Event-driven S-expression parser.
    Fires callbacks when nodes open and close.
    """

    def __init__(
        self,
        content: str,
        on_node_open: Optional[CallbackFn] = None,
        on_node_close: Optional[CallbackFn] = None,
    ):
        self.content = content
        self.on_node_open = on_node_open
        self.on_node_close = on_node_close
        self.stack = []

    def parse(self) -> None:
        """Parse the content string and trigger registered callbacks."""
        tokens = self._tokenize(self.content)

        for token_type, value, line_num in tokens:
            if token_type == "LPAREN":
                # Push a new node frame onto the stack
                frame = {"type": None, "line": line_num, "atoms": []}
                self.stack.append(frame)

            elif token_type == "RPAREN":
                if not self.stack:
                    continue  # Guard against malformed S-expressions

                current_frame = self.stack.pop()
                node_type = (
                    current_frame["type"] if current_frame["type"] else "UNKNOWN"
                )

                if self.on_node_close:
                    self.on_node_close(current_frame, line_num)

            elif token_type == "ATOM":
                if self.stack:
                    current_frame = self.stack[-1]
                    # The first atom immediately after '(' defines the node type
                    if current_frame["type"] is None:
                        current_frame["type"] = value
                        if self.on_node_open:
                            self.on_node_open(current_frame, line_num)
                    else:
                        current_frame["atoms"].append(value)

    def _tokenize(
        self, text: str
    ) -> Generator[Tuple[str, str, int], None, None]:
        """
        Tokenizer yielding (token_type, value, line_number).
        Handles string literals, parentheses, and S-expression atoms.
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
            elif char == "(":
                yield ("LPAREN", "(", line_num)
                i += 1
            elif char == ")":
                yield ("RPAREN", ")", line_num)
                i += 1
            elif char == '"':
                # Quoted string literal token
                start = i
                i += 1
                while i < n and text[i] != '"':
                    if text[i] == "\\" and i + 1 < n:
                        i += 2  # Skip escaped characters
                    else:
                        if text[i] == "\n":
                            line_num += 1
                        i += 1
                if i < n:
                    i += 1  # Include closing quote
                yield ("ATOM", text[start:i], line_num)
            else:
                # Unquoted atom
                start = i
                while (
                    i < n and not text[i].isspace() and text[i] not in "()"
                ):
                    i += 1
                yield ("ATOM", text[start:i], line_num)
