import argparse
from enum import Enum
from .trace_generator import generate_spiral_trace
from .kicad_footprint_generator import generate_kicad_footprint
from .pcb_trace_visualization import visualize_pcb_trace
from .magnetic_field_simulation import MagneticFieldSimulation


class Direction(Enum):
    X_AXIS = "x"
    Y_AXIS = "y"
    Z_AXIS = "z"


# Default parameters that match the capabilities of JLCPCB.
# The final parameters used should be verified against https://jlcpcb.com/capabilities/pcb-capabilities
DEFAULTS = {
    "x_radius": 0.1, # 10cm
    "y_radius": 0.1, # 10cm
    "layers": 2,
    "pad_angle": 2.35619449019, # Start footprint in upper left corner.
    "max_trace_width": 1e-3,  # 1.0 mm
    "min_trace_width": 0.1e-3,  # 0.1 mm
    "outer_trace_thickness": 35e-6,  # 35 µm (1 oz copper, standard weight of outer layers)
    "inner_trace_thickness": 17.5e-6,  # 17.5 µm (0.5 oz copper, standard weight of inner layers)
    "trace_spacing": 0.15e-3,  # 0.15 mm spacing between traces
    "width_exponent": 1.0,
    "output": "magnetorquer.kicad_mod",
    "show_visualization": False,
    "estimate_force": False,
    "magnetic_field_strength": 25e-6, # 25uT, value of Earth's magnetic field in LEO near the equator.
    "magnetic_field_direction": Direction.Z_AXIS,
    "voltage": 3.3,
}


def main():
    parser = argparse.ArgumentParser(
        description="Generate a KiCad footprint for a spiral magnetorquer",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("--x-radius", type=float, default=DEFAULTS["x_radius"], help="Radius of spiral in x-direction in meters.")
    parser.add_argument("--y-radius", type=float, default=DEFAULTS["y_radius"], help="Radius of spiral in y-direction meters.")
    parser.add_argument("--layers", type=int, default=DEFAULTS["layers"], help="Number of layers")
    parser.add_argument("--pad-angle", type=float, default=DEFAULTS["pad_angle"], help="Angular placement of start and end pads in radians")
    parser.add_argument("--max-trace-width", type=float, default=DEFAULTS["max_trace_width"], help="Maximum trace width in meters")
    parser.add_argument("--min-trace-width", type=float, default=DEFAULTS["min_trace_width"], help="Minimum trace width in meters")
    parser.add_argument("--outer-trace-thickness", type=float, default=DEFAULTS["outer_trace_thickness"], help="Trace thickness of outer layers (F.Cu and B.Cu) in meters")
    parser.add_argument("--inner-trace-thickness", type=float, default=DEFAULTS["inner_trace_thickness"], help="Trace thickness of inner layers (In*.Cu) in meters")
    parser.add_argument("--trace-spacing", type=float, default=DEFAULTS["trace_spacing"], help="Minimum spacing between traces")
    parser.add_argument("--width-exp", type=float, default=DEFAULTS["width_exponent"], help="Exponent controlling width variation")
    parser.add_argument("--output", type=str, default=DEFAULTS["output"], help="Output .kicad_mod file")
    parser.add_argument("--show-visualization", action='store_true', default=DEFAULTS["show_visualization"], help="Show a visualization of the PCBTrace after generating")
    parser.add_argument("--estimate-force", action='store_true', default=DEFAULTS["estimate_force"], help="Estimate the force and torque on the board by a constant magnetic field")
    parser.add_argument("--magnetic-field-strength", type=float, default=DEFAULTS["magnetic_field_strength"], help="Strength of external magnetic field for force and torque estimates. (Tesla)")
    parser.add_argument("--magnetic-field-direction", type=Direction, choices=list(Direction), default=DEFAULTS["magnetic_field_direction"], help="Direction of external magnetic field for force and torque estimates")
    parser.add_argument("--voltage", type=float, default=DEFAULTS["voltage"], help="Voltage across trace of magnetorquer (Volts)")

    args = parser.parse_args()

    # Compute spiral parameters
    center = (0.0, 0.0)

    trace = generate_spiral_trace(
        center=center,
        x_radius=args.x_radius,
        y_radius=args.y_radius,
        layers=args.layers,
        pad_angle=args.pad_angle,
        max_trace_width=args.max_trace_width,
        min_trace_width=args.min_trace_width,
        trace_width_exponent=args.width_exp,
        trace_spacing=args.trace_spacing,
        trace_thickness_outer_layers=args.outer_trace_thickness,
        trace_thickness_inner_layers=args.inner_trace_thickness,
    )

    print(f"Trace resistance: {trace.estimate_resistance()}")

    # Export as KiCad footprint
    generate_kicad_footprint(trace, args.output)
    print(f"Footprint written to {args.output}")

    if args.estimate_force:
        print("Simulating magnetic field")
        sim = MagneticFieldSimulation(trace, args.voltage)
        if args.magnetic_field_direction == Direction.X_AXIS:
            B_ext = (args.magnetic_field_strength, 0, 0)
        elif args.magnetic_field_direction == Direction.Y_AXIS:
            B_ext = (0, args.magnetic_field_strength, 0)
        else:
            B_ext = (0, 0, args.magnetic_field_strength)
        force, torque = sim.compute_force_and_torque(B_ext, (0, 0, 0))
        print(f"\nTotal Force on PCB: {force} N")
        print(f"Total Torque on PCB: {torque} N·m")

    if args.show_visualization:
        visualize_pcb_trace(trace)


if __name__ == "__main__":
    main()
