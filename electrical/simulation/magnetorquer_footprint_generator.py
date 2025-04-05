import argparse
from .trace_generator import generate_spiral_trace
from .kicad_footprint_generator import generate_kicad_footprint
from .pcb_trace_visualization import visualize_pcb_trace

# Default parameters that match the capabilities of JLCPCB.
# The final parameters used should be verified against https://jlcpcb.com/capabilities/pcb-capabilities
DEFAULTS = {
    "size": 0.1,  # meters (10 cm)
    "radius": -1, # Compute from size
    "layers": 2,
    "pad_angle": 0,
    "max_trace_width": 1e-3,  # 1.0 mm
    "min_trace_width": 0.1e-3,  # 0.1 mm
    "outer_trace_thickness": 35e-6,  # 35 µm (1 oz copper, standard weight of outer layers)
    "inner_trace_thickness": 17.5e-6,  # 17.5 µm (0.5 oz copper, standard weight of inner layers)
    "trace_spacing": 0.09e-3,  # 0.09 mm spacing between traces
    "width_exponent": 1.0,
    "output": "magnetorquer.kicad_mod",
    "show_visualization": False,
}


def main():
    parser = argparse.ArgumentParser(
        description="Generate a KiCad footprint for a spiral magnetorquer",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("--size", type=float, default=DEFAULTS["size"], help="PCB size in meters (square). Ignored if max_radius is specified.")
    parser.add_argument("--radius", type=float, default=DEFAULTS["radius"], help="Radius of spiral in meters. Computed from size by default.")
    parser.add_argument("--layers", type=int, default=DEFAULTS["layers"], help="Number of layers")
    parser.add_argument("--pad_angle", type=int, default=DEFAULTS["pad_angle"], help="Angular placement of start and end pads in radians")
    parser.add_argument("--max-trace-width", type=float, default=DEFAULTS["max_trace_width"], help="Maximum trace width in meters")
    parser.add_argument("--min-trace-width", type=float, default=DEFAULTS["min_trace_width"], help="Minimum trace width in meters")
    parser.add_argument("--outer_trace_thickness", type=float, default=DEFAULTS["outer_trace_thickness"], help="Trace thickness of outer layers (F.Cu and B.Cu) in meters")
    parser.add_argument("--inner_trace_thickness", type=float, default=DEFAULTS["inner_trace_thickness"], help="Trace thickness of inner layers (In*.Cu) in meters")
    parser.add_argument("--trace-spacing", type=float, default=DEFAULTS["trace_spacing"], help="Minimum spacing between traces")
    parser.add_argument("--width-exp", type=float, default=DEFAULTS["width_exponent"], help="Exponent controlling width variation")
    parser.add_argument("--output", type=str, default=DEFAULTS["output"], help="Output .kicad_mod file")
    parser.add_argument("--show_visualization", action='store_true', default=DEFAULTS["show_visualization"], help="Show a visualization of the PCBTrace after generating")

    args = parser.parse_args()

    # Compute spiral parameters
    center = (0.0, 0.0)
    if args.radius < 0.0:
        radius = args.size / 2 * 0.9  # Keep margin inside board
    else:
        radius = args.radius

    trace = generate_spiral_trace(
        center=center,
        radius=radius,
        layers=args.layers,
        pad_angle=args.pad_angle,
        max_trace_width=args.max_trace_width,
        min_trace_width=args.min_trace_width,
        trace_width_exponent=args.width_exp,
        trace_spacing=args.trace_spacing,
        trace_thickness_outer_layers=args.outer_trace_thickness,
        trace_thickness_inner_layers=args.inner_trace_thickness,
    )

    print(f"Trace resistance: {trace.total_resistance()}")

    # Export as KiCad footprint
    generate_kicad_footprint(trace, args.output)
    print(f"Footprint written to {args.output}")

    if args.show_visualization:
        visualize_pcb_trace(trace)


if __name__ == "__main__":
    main()
