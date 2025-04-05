import math
import numpy as np
from typing import List, Tuple
from .pcb_trace import PCBTrace, TraceSegment, Via

def generate_spiral_between_points(
    trace: PCBTrace,
    start: Tuple[float, float],
    end: Tuple[float, float],
    center: Tuple[float, float],
    trace_spacing: float,
    max_trace_width: float,
    min_trace_width: float,
    trace_width_exponent: float,
    trace_thickness: float,
    current_layer: int,
    chirality: str,  # "cw" or "ccw"
    angle_step: float,
):
    """
    Generates a smooth spiral trace from start to end around a center point.

    All linear units are meters. All angular units are radians.

    Parameters:
        trace: PCBTrace to append to. 
        start: (x, y) start point.
        end: (x, y) end point.
        center: (x, y) spiral center.
        trace_spacing: radial distance between spiral arms.
        max_trace_width: Maximum allowed width for the trace.
        min_trace_width: Minimum allowed width for the trace.
        trace_width_exponent: controls how width varies with radius.
        trace_thickness: copper thickness on the this layer.
        chirality: "cw" or "ccw" — determines spiral turning direction.
        angle_step: angular resolution in radians.
    """
    assert chirality in ("cw", "ccw"), "Chirality must be 'cw' or 'ccw'."

    cx, cy = center
    sx, sy = start
    ex, ey = end

    # Start/end in polar coords
    dx0, dy0 = sx - cx, sy - cy
    dx1, dy1 = ex - cx, ey - cy
    r0 = math.hypot(dx0, dy0)
    r1 = math.hypot(dx1, dy1)
    theta0 = math.atan2(dy0, dx0)
    theta1 = math.atan2(dy1, dx1)

    # Determine angular direction
    delta_theta = theta1 - theta0

    # Unwrap to minimal signed angle
    while delta_theta <= -math.pi:
        delta_theta += 2 * math.pi
    while delta_theta > math.pi:
        delta_theta -= 2 * math.pi

    # Set chirality direction
    direction = 1 if chirality == "ccw" else -1

    # Set up trace_width to be a constant across the entire spiral.
    # TODO(james): Modify this code to support a trace_width that changes according to the radius.
    trace_width = min_trace_width
    
    # Calculate total revolutions (ensure spiral, not arc)
    radial_diff = abs(r1 - r0)
    num_turns = max(1, int(radial_diff / (trace_spacing + trace_width)))
    extra_angle = 2 * math.pi * num_turns * direction

    # Total angle to sweep
    theta_final = theta0 + extra_angle + delta_theta
    total_angle = theta_final - theta0

    # Steps and increments
    steps = max(2, int(abs(total_angle / angle_step)))
    dtheta = direction * total_angle / steps
    dr = (r1 - r0) / steps

    # Generate line segments
    prev_point = None
    curr_point = None
    r = r0
    theta = theta0
    for _ in range(steps):
        x = cx + r * math.cos(theta)
        y = cy + r * math.sin(theta)
        curr_point = (x, y)
        if prev_point != None:
            trace.add_segment(TraceSegment(start=prev_point, end=curr_point, width=trace_width, layer=current_layer, thickness=trace_thickness))
        theta += dtheta
        r += dr
        prev_point = curr_point

    if prev_point != None:
        trace.add_segment(TraceSegment(start=prev_point, end=end, width=trace_width, layer=current_layer, thickness=trace_thickness))


def generate_spiral_trace(
    center: Tuple[float, float],
    radius: float,
    layers: int,
    pad_angle: float,
    max_trace_width: float,
    min_trace_width: float,
    trace_width_exponent: float = 1.0,
    trace_spacing: float = 0.2e-3,
    trace_thickness_outer_layers: float = 35e-6,
    trace_thickness_inner_layers: float = 18e-6,
    angle_step: float = 0.05,
) -> PCBTrace:
    """
    Generate a PCBTrace that spirals inward and outward over multiple layers.

    The spiral starts at the outer radius and spirals inward, switching layers via
    thru vias and spiraling back outward, continuing the pattern until all layers are used.

    Parameters:
        center: (x, y) center of the spiral.
        radius: maximum radius of the spiral from center.
        layers: total number of PCB layers.
        pad_angle: angle in radians to locate the start/end pads.
        max_trace_width: width at the outer edge of the spiral.
        min_trace_width: width at the center of the spiral.
        trace_width_exponent: controls how trace width varies with radius.
        trace_spacing: radial spacing between spiral turns.
        trace_thickness_outer_layers: trace thickness on outer layers.
        trace_thickness_inner_layers: trace thickness on inner layers.
        angle_step: resolution of angle steps in radians.

    Returns:
        PCBTrace instance with segments and vias.
    """
    trace = PCBTrace(layers=layers)

    # Calculate start/end point at max radius
    x0 = center[0] + radius * math.cos(pad_angle)
    y0 = center[1] + radius * math.sin(pad_angle)
    start_point = (x0, y0)
    current_point = start_point

    # Generate spirals for each layer
    for i in range(layers):
        next_radius = radius * 0.1  # a small inner radius to spiral toward (then reverse)
        if i % 2 == 1:
            # Odd layer: spiral outward
            next_radius = radius
            end_point = (
                center[0] + next_radius * math.cos(pad_angle),
                center[1] + next_radius * math.sin(pad_angle)
            )
        else:
            # Even layer: spiral inward
            next_radius = radius * 0.1
            end_point = (
                center[0] + next_radius * math.cos(pad_angle),
                center[1] + next_radius * math.sin(pad_angle)
            )

        chirality = "ccw" if i % 2 == 0 else "cw"
        trace_thickness = (
            trace_thickness_outer_layers if i == 0 or i == layers - 1 else trace_thickness_inner_layers
        )

        generate_spiral_between_points(
            trace=trace,
            start=current_point,
            end=end_point,
            center=center,
            trace_spacing=trace_spacing,
            max_trace_width=max_trace_width,
            min_trace_width=min_trace_width,
            trace_width_exponent=trace_width_exponent,
            trace_thickness=trace_thickness,
            current_layer=i,
            chirality=chirality,
            angle_step=angle_step,
        )

        current_point = end_point

        # Add via to next layer (unless on final layer)
        if i < layers - 1:
            via = Via(position=current_point)
            trace.vias.append(via)

    # Connect final point back to start
    if current_point != start_point:
        final_via = Via(position=start_point)
        trace.vias.append(final_via)

    return trace
