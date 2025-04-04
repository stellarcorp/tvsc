import math
import numpy as np
from .pcb_trace import PCBTrace, TraceSegment, Via

def generate_spiral_trace(
    center,
    max_radius,
    min_radius,
    turns,
    layers,
    max_trace_width,
    min_trace_width,
    trace_thickness_outer_layers,
    trace_thickness_inner_layers,
    trace_spacing,
    trace_width_exponent=1.0,
    start_angle=0.0,
    angle_step=0.01,
):
    """
    Generates a spiral PCBTrace that starts at max_radius and spirals inward, 
    switches layers via a thru-via, and spirals outward, repeating across all layers.

    Parameters:
        center: (x, y) tuple of spiral center (meters)
        max_radius: outermost spiral radius (meters)
        min_radius: innermost radius to reverse direction (meters)
        turns: number of turns in each layer
        layers: number of copper layers
        max_trace_width: maximum trace width (meters)
        min_trace_width: minimum trace width (meters)
        trace_thickness_outer_layers: copper thickness on the outer layers F.Cu and B.Cu (meters)
        trace_thickness_inner_layers: copper thickness on the inner layers In*.Cu (meters)
        spacing: minimum spacing between traces (meters)
        trace_width_exponent: controls how width varies with radius
        start_angle: starting angle of spiral (radians)
        angle_step: angular step for each trace segment (radians)

    Returns:
        PCBTrace instance representing the full spiral magnetorquer
    """
    layers = 2 * (layers // 2) # An odd number of layers can't be manufactured.
    delta_radius_per_turn = (max_radius - min_radius) / turns
    delta_radius = delta_radius_per_turn * angle_step / (2 * math.pi)
    cx, cy = center
    trace = PCBTrace(layers=layers)
    direction = -1  # -1 for inward, 1 for outward
    angle = start_angle
    radius = max_radius
    prev_point = None

    for layer in range(layers):
        is_last_layer = (layer == layers - 1)

        if layer == 0 or layer == layers - 1:
            trace_thickness = trace_thickness_outer_layers
        else:
            trace_thickness = trace_thickness_inner_layers

        if direction == -1:
            radius_start = max_radius
            radius_end = min_radius
        else:
            radius_start = min_radius
            radius_end = max_radius

        radius = radius_start
        delta_radius *= -1  # Change directions inward/outward

        total_steps = int(turns * 2 * math.pi / abs(angle_step))
        for _ in range(total_steps):
            trace_progress = abs((max_radius - radius) / (max_radius - min_radius))
            trace_width = min_trace_width + (max_trace_width - min_trace_width) * (trace_progress ** trace_width_exponent)

            x = cx + radius * math.cos(angle)
            y = cy + radius * math.sin(angle)
            point = (x, y)

            if prev_point is not None:
                trace.add_segment(TraceSegment(
                    start=prev_point,
                    end=point,
                    width=trace_width,
                    layer=layer,
                    thickness=trace_thickness
                ))
            prev_point = point

            radius += delta_radius
            angle += angle_step

        if not is_last_layer:
            # TODO(james): Insert via offset to prevent short circuits
            via = Via(
                position=(prev_point[0], prev_point[1]),
                start_layer=layer,
                end_layer=layer + 1
            )
            trace.add_via(via)

        # Reverse spiral direction and continue
        direction *= -1
        angle += math.pi / 2

    return trace
