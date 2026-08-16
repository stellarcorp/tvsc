#!/usr/bin/env python3
"""
KiCad UUID generation utility and library.
"""

import uuid
from typing import List, Union


def generate_uuid() -> str:
    """Generate a single random UUID4 formatted as a lowercase string."""
    return str(uuid.uuid4()).lower()


def generate_deterministic_uuid(namespace_seed: str, name: str) -> str:
    """
    Generate a deterministic UUID5 based on a seed string and name string.
    Useful for creating stable UUIDs for design blocks or sub-sheets.
    """
    ns = uuid.uuid5(uuid.NAMESPACE_DNS, namespace_seed)
    return str(uuid.uuid5(ns, name)).lower()


def main():
    import argparse
    import sys

    parser = argparse.ArgumentParser(
        description="KiCad-compatible UUID generator",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "-s",
        "--seed",
        type=str,
        default="stellarcorp.tv",
        help="Optional seed for deterministic UUID5 generation",
    )
    parser.add_argument(
        "--name",
        type=str,
        help="Name string used for deterministic generation",
    )

    args = parser.parse_args()

    try:
        if args.name:
            print(generate_deterministic_uuid(args.seed, args.name))
        else:
            print(generate_uuid())
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
