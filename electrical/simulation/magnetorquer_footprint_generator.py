import argparse
import numpy as np
from enum import Enum
from .trace_generator import generate_spiral_trace
from .kicad_footprint_generator import generate_kicad_footprint
from .pcb_trace_visualization import visualize_pcb
from .magnetic_field_simulation import MagneticFieldSimulation
from .pcb_trace import PCB


class Direction(Enum):
    X_AXIS = "x"
    Y_AXIS = "y"
    Z_AXIS = "z"


DEFAULTS = {
    "x_size": 0.1, # 10cm
    "y_size": 0.1, # 10cm
    "x_radius": 0.04, # 4cm
    "y_radius": 0.04, # 4cm
    "squareness": 0.0, # squircle squareness. 0 -> circle, 1-> square.
    "inner_radius": 0.0,
    "layers": 2,
    "pad_angle": 2.35619449019, # Start footprint in upper left corner.
    "width_exponent": 1.0,
    "output": "magnetorquer.kicad_mod",
    "show_visualization": False,
    "estimate_force": False,
    "magnetic_field_strength": 25e-6, # 25uT, lower bound of Earth's magnetic field in LEO near the equator.
    "magnetic_field_direction": Direction.Z_AXIS,
    "voltage": 3.3,
    "board_thickness": 0.00157, # 1.6mm (nominal) is the standard PCB thickness.

    # Default parameters that match the capabilities of JLCPCB with some modifications based on cost.
    # The final parameters used should be verified against https://jlcpcb.com/capabilities/pcb-capabilities
    "max_trace_width": 1e-3,  # 1.0 mm
    "min_trace_width": 0.1e-3,  # 0.1 mm
    "trace_thickness_outer_layers": 35e-6,  # 35 µm (1 oz copper, standard weight of outer layers)
    "trace_thickness_inner_layers": 17.5e-6,  # 17.5 µm (0.5 oz copper, standard weight of inner layers)
    "trace_spacing": 0.15e-3,  # 0.15 mm spacing between traces
    "min_via_drill_size": 0.0003, # 0.3 mm, chosen for cost
    "min_via_diameter": 0.00045, # 0.45 mm, chosen for cost
    "max_current": 1., # Max continuous current for the magnetorquer driver IC.
}


def main():
    parser = argparse.ArgumentParser(
        description="Generate a KiCad footprint for a spiral magnetorquer",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )

    parser.add_argument("--output", type=str, default=DEFAULTS["output"], help="Output .kicad_mod file")
    parser.add_argument("--show-visualization", action='store_true', default=DEFAULTS["show_visualization"], help="Show a visualization of the PCBTrace after generating")
    parser.add_argument("--estimate-force", action='store_true', default=DEFAULTS["estimate_force"], help="Estimate the force and torque on the board by a constant magnetic field")
    parser.add_argument("--x-size", type=float, default=DEFAULTS["x_size"], help="Size of PCB in x-direction in meters.")
    parser.add_argument("--y-size", type=float, default=DEFAULTS["y_size"], help="Size of PCB in y-direction in meters.")
    parser.add_argument("--board-thickness", type=float, default=DEFAULTS["board_thickness"], help="Thickness of the PCB in meters. 1.57mm is the most common thickness. JLCPCB can manufacture boards from 0.4mm to 4.5mm.")
    parser.add_argument("--layers", type=int, default=DEFAULTS["layers"], help="Number of layers")

    parser.add_argument("--squareness", type=float, default=DEFAULTS["squareness"], help="Squareness of the squircle.")
    parser.add_argument("--x-radius", type=float, default=DEFAULTS["x_radius"], help="Radius of spiral in x-direction in meters.")
    parser.add_argument("--y-radius", type=float, default=DEFAULTS["y_radius"], help="Radius of spiral in y-direction meters.")
    parser.add_argument("--inner-radius", type=float, default=DEFAULTS["inner_radius"], help="Radius of the innermost spiral (both x- and y-directions) in meters.")
    parser.add_argument("--pad-angle", type=float, default=DEFAULTS["pad_angle"], help="Angular placement of start and end pads in radians")
    parser.add_argument("--width-exp", type=float, default=DEFAULTS["width_exponent"], help="Exponent controlling width variation")

    parser.add_argument("--magnetic-field-strength", type=float, default=DEFAULTS["magnetic_field_strength"], help="Strength of external magnetic field for force and torque estimates. (Tesla)")
    parser.add_argument("--magnetic-field-direction", type=Direction, choices=list(Direction), default=DEFAULTS["magnetic_field_direction"], help="Direction of external magnetic field for force and torque estimates")
    parser.add_argument("--voltage", type=float, default=DEFAULTS["voltage"], help="Voltage across trace of magnetorquer (Volts)")

    parser.add_argument("--max-trace-width", type=float, default=DEFAULTS["max_trace_width"], help="Maximum trace width in meters")
    parser.add_argument("--min-trace-width", type=float, default=DEFAULTS["min_trace_width"], help="Minimum trace width in meters")
    parser.add_argument("--trace-thickness-outer-layers", type=float, default=DEFAULTS["trace_thickness_outer_layers"], help="Trace thickness of outer layers (F.Cu and B.Cu) in meters")
    parser.add_argument("--trace-thickness-inner-layers", type=float, default=DEFAULTS["trace_thickness_inner_layers"], help="Trace thickness of inner layers (In*.Cu) in meters")
    parser.add_argument("--trace-spacing", type=float, default=DEFAULTS["trace_spacing"], help="Minimum spacing between traces, vias, and pads.")
    parser.add_argument("--min-via-drill-size", type=float, default=DEFAULTS["min_via_drill_size"], help="Minimum drill diameter for vias.")
    parser.add_argument("--min-via-diameter", type=float, default=DEFAULTS["min_via_diameter"], help="Minimum via diameter. Includes both the pad and hole.")
    parser.add_argument("--max-current", type=float, default=DEFAULTS["max_current"], help="Maximum continuous current that can be handled by the magnetorquer and its driver IC.")

    args = parser.parse_args()

    pcb = PCB()
    pcb.size = np.array([args.x_size, args.y_size, args.board_thickness])
    pcb.layers=args.layers
    pcb.constraints.max_trace_width = args.max_trace_width
    pcb.constraints.min_trace_width = args.min_trace_width
    pcb.constraints.trace_thickness_outer_layers = args.trace_thickness_outer_layers
    pcb.constraints.trace_thickness_inner_layers = args.trace_thickness_inner_layers
    pcb.constraints.trace_spacing = args.trace_spacing
    pcb.constraints.min_via_drill_size = args.min_via_drill_size
    pcb.constraints.min_via_diameter = args.min_via_diameter
    pcb.constraints.max_current = args.max_current

    # Compute spiral parameters
    generate_spiral_trace(
        pcb,
        squareness=args.squareness,
        x_radius=args.x_radius,
        y_radius=args.y_radius,
        inner_radius=args.inner_radius,
        pad_angle=args.pad_angle,
        trace_width_exponent=args.width_exp,
    )

    for net in pcb.nets:
        resistance = net.estimate_resistance()
        voltage = args.voltage
        calculated_current = voltage / resistance
        print(f"Net resistance: {resistance} ohm")
        if calculated_current > pcb.constraints.max_current:
            current = pcb.constraints.max_current
            voltage = current * resistance
            print(f"Current @{voltage} V: {current} A (Note: constrained by magnetorquer current limitations)")
        else:
            current = calculated_current
            print(f"Current @{voltage} V: {current} A")

    # Export as KiCad footprint
    generate_kicad_footprint(pcb, args.output)
    print(f"Footprint written to {args.output}")

    if args.estimate_force:
        print("Simulating magnetic field")
        for net in pcb.nets:
            sim = MagneticFieldSimulation(net, args.voltage, pcb.constraints.max_current)
            if args.magnetic_field_direction == Direction.X_AXIS:
                B_ext = np.array([args.magnetic_field_strength, 0., 0.])
            elif args.magnetic_field_direction == Direction.Y_AXIS:
                B_ext = np.array([0., args.magnetic_field_strength, 0.])
            else:
                B_ext = np.array([0., 0., args.magnetic_field_strength])
            force = sim.compute_force(B_ext)
            torque = sim.compute_torque(B_ext)
        print(f"\nTotal Force on PCB: {force} N")
        print(f"Total Torque on PCB: {torque} N·m")

    if args.show_visualization:
        visualize_pcb(pcb)


if __name__ == "__main__":
    main()
