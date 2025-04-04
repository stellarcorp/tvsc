import argparse
from .trace_generator import generate_spiral_trace
from .kicad_footprint_generator import generate_kicad_footprint

# Default parameters for a 10x10 cm, 6-layer FR-4 PCB from JLCPCB
DEFAULTS = {
    "size": 0.1,  # meters (10 cm)
    "layers": 6,
    "turns": 20,
    "max_width": 0.5e-3,  # 0.5 mm
    "min_width": 0.2e-3,  # 0.2 mm
    "trace_thickness": 35e-6,  # 35 µm (standard 1 oz copper)
    "spacing": 0.2e-3,  # 0.2 mm clearance
    "width_exponent": 1.0,
    "output": "magnetorquer.kicad_mod",
}


def main():
    parser = argparse.ArgumentParser(description="Generate a KiCad footprint for a spiral magnetorquer")
    parser.add_argument("--size", type=float, default=DEFAULTS["size"], help="PCB size in meters (square)")
    parser.add_argument("--layers", type=int, default=DEFAULTS["layers"], help="Number of layers (default: 6)")
    parser.add_argument("--turns", type=int, default=DEFAULTS["turns"], help="Number of spiral turns")
    parser.add_argument("--max-width", type=float, default=DEFAULTS["max_width"], help="Maximum trace width in meters")
    parser.add_argument("--min-width", type=float, default=DEFAULTS["min_width"], help="Minimum trace width in meters")
    parser.add_argument("--thickness", type=float, default=DEFAULTS["trace_thickness"], help="Trace thickness in meters")
    parser.add_argument("--spacing", type=float, default=DEFAULTS["spacing"], help="Minimum spacing between traces")
    parser.add_argument("--width-exp", type=float, default=DEFAULTS["width_exponent"], help="Exponent controlling width variation")
    parser.add_argument("--output", type=str, default=DEFAULTS["output"], help="Output .kicad_mod file")

    args = parser.parse_args()

    # Compute spiral parameters
    center = (0.0, 0.0)
    max_radius = args.size / 2 * 0.9  # Keep margin inside board
    trace = generate_spiral_trace(
        center=center,
        max_radius=max_radius,
        turns=args.turns,
        layers=args.layers,
        max_width=args.max_width,
        min_width=args.min_width,
        trace_thickness=args.thickness,
        spacing=args.spacing,
        width_exponent=args.width_exp,
    )

    # Export as KiCad footprint
    generate_kicad_footprint(trace, args.output)
    print(f"Footprint written to {args.output}")


if __name__ == "__main__":
    main()
