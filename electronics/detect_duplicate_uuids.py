#!/usr/bin/env python3

import sys
from pathlib import Path
from collections import defaultdict


class SExprParser:
    """
    Event-driven S-expression parser using a stack to maintain node context.
    """
    def __init__(self, content: str):
        self.content = content
        self.stack = []
        self.uuid_map = defaultdict(list)  # uuid -> list of node info dicts

    def parse(self, file_path: Path):
        tokens = self._tokenize(self.content)
        
        for token_type, value, line_num in tokens:
            if token_type == 'LPAREN':
                # Push a new node frame onto the stack: [node_type, line_number, children_data]
                self.stack.append({
                    'type': None,
                    'line': line_num,
                    'atoms': []
                })
            elif token_type == 'RPAREN':
                if not self.stack:
                    continue  # Malformed S-expr guard
                
                # Pop the current node
                current_node = self.stack.pop()
                node_type = current_node['type'] if current_node['type'] else 'UNKNOWN'
                
                # Execute closing handler
                self._on_node_close(node_type, current_node, file_path)
            elif token_type == 'ATOM':
                if self.stack:
                    # If this is the first atom inside an open paren, it defines the node type
                    if self.stack[-1]['type'] is None:
                        self.stack[-1]['type'] = value
                    else:
                        self.stack[-1]['atoms'].append(value)

        return self.uuid_map

    def _on_node_close(self, node_type: str, current_node: dict, file_path: Path):
        """
        Closing handler executed when a node closes.
        """
        # Specific closing handler for UUID nodes
        if node_type == 'uuid':
            uuid_val = current_node['atoms'][0] if current_node['atoms'] else None
            if not uuid_val:
                return

            # Clean quotes if present
            uuid_val = uuid_val.strip('"').lower()

            # Inspect the parent node on the stack to get context
            if self.stack:
                parent_node = self.stack[-1]
                parent_type = parent_node['type'] if parent_node['type'] else 'root'
                parent_line = parent_node['line']
            else:
                parent_type = 'file_root'
                parent_line = current_node['line']

            # Record UUID to node mapping
            self.uuid_map[uuid_val].append({
                'file': file_path,
                'node_type': parent_type,
                'line': parent_line
            })

    def _tokenize(self, text: str):
        """
        Tokenizer yielding (token_type, value, line_number).
        Handles string literals, parentheses, and S-expression atoms.
        """
        i = 0
        n = len(text)
        line_num = 1

        while i < n:
            char = text[i]

            if char == '\n':
                line_num += 1
                i += 1
            elif char.isspace():
                i += 1
            elif char == '(':
                yield ('LPAREN', '(', line_num)
                i += 1
            elif char == ')':
                yield ('RPAREN', ')', line_num)
                i += 1
            elif char == '"':
                # Quoted string literal token
                start = i
                i += 1
                while i < n and text[i] != '"':
                    if text[i] == '\\' and i + 1 < n:
                        i += 2  # Skip escaped characters
                    else:
                        if text[i] == '\n':
                            line_num += 1
                        i += 1
                if i < n:
                    i += 1  # Include closing quote
                yield ('ATOM', text[start:i], line_num)
            else:
                # Unquoted atom
                start = i
                while i < n and not text[i].isspace() and text[i] not in '()':
                    i += 1
                yield ('ATOM', text[start:i], line_num)


def parse_project_files(project_dir: Path):
    """
    Parses all S-expression files in a project directory into a combined UUID map.
    """
    extensions = {".kicad_sch", ".kicad_pcb", ".kicad_pro", ".kicad_wks"}
    combined_uuid_map = defaultdict(list)

    for file_path in project_dir.rglob("*"):
        if file_path.is_file() and file_path.suffix in extensions:
            try:
                content = file_path.read_text(encoding="utf-8", errors="ignore")
                parser = SExprParser(content)
                file_uuid_map = parser.parse(file_path.relative_to(project_dir))

                for uuid_val, occurrences in file_uuid_map.items():
                    combined_uuid_map[uuid_val].extend(occurrences)
            except Exception as e:
                print(f"Error parsing {file_path}: {e}", file=sys.stderr)

    return combined_uuid_map


def generate_intersection_report(proj1_dir: Path, proj2_dir: Path):
    proj1_map = parse_project_files(proj1_dir)
    proj2_map = parse_project_files(proj2_dir)

    shared_uuids = set(proj1_map.keys()) & set(proj2_map.keys())

    # Sort priorities: highest priority nodes go to the bottom of the report
    # (so they are immediately visible above the terminal prompt).
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
        # Rank UUID by its highest-priority parent node across both projects
        nodes = proj1_map[uuid_val] + proj2_map[uuid_val]
        return max(type_priority.get(node["node_type"], 0) for node in nodes)

    # Sort UUIDs: low-priority types first, critical types (sheet, pcb, pro) at the end
    sorted_uuids = sorted(shared_uuids, key=lambda u: (get_uuid_priority(u), u))

    for uuid_val in sorted_uuids:
        # Combine occurrences from both projects
        occurrences = []
        for occ in proj1_map[uuid_val]:
            occurrences.append((proj1_dir / occ["file"], occ["line"], occ["node_type"]))
        for occ in proj2_map[uuid_val]:
            occurrences.append((proj2_dir / occ["file"], occ["line"], occ["node_type"]))

        # Check if types differ across uses
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
        print("Usage: python detect_duplicate_uuids.py <path/to/project1> <path/to/project2>")
        sys.exit(1)

    p1 = Path(sys.argv[1])
    p2 = Path(sys.argv[2])

    if not p1.is_dir() or not p2.is_dir():
        print("Error: Both arguments must be valid directories.", file=sys.stderr)
        sys.exit(1)

    generate_intersection_report(p1, p2)
