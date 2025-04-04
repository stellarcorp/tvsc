import numpy as np
from .pcb_trace import PCBTrace, Segment, Via

def generate_spiral_trace(center, max_radius, turns, layers, max_width, min_width, trace_thickness, spacing, width_exponent):
    """
    Generates a spiral PCB trace that starts at the maximum radius and spirals inward first,
    then spirals outward to end near the starting point. All layer transitions use thru vias.

    Parameters:
        center (tuple): (x, y) center of the spiral.
        max_radius (float): Maximum radius of the spiral.
        turns (int): Number of turns in the spiral.
        layers (int): Number of PCB layers used.
        max_width (float): Maximum trace width.
        min_width (float): Minimum trace width.
        trace_thickness (float): Thickness of the copper trace.
        spacing (float): Minimum spacing between traces.
        width_exponent (float): Controls how trace width varies along the spiral.

    Returns:
        PCBTrace
    """
    segments = []
    vias = []
    current_radius = max_radius  # Start at the outermost radius
    angle = 0
    delta_angle = np.pi / 8  # Small angle step for smooth spirals
    current_layer = 0

    # Spiral inward
    for turn in range(turns // 2):
        while current_radius > max_width / 2:
            trace_width = min_width + (max_width - min_width) * (current_radius / max_radius) ** width_exponent
            next_radius = current_radius - (trace_width + spacing)
            next_angle = angle + delta_angle

            x1, y1 = center[0] + current_radius * np.cos(angle), center[1] + current_radius * np.sin(angle)
            x2, y2 = center[0] + next_radius * np.cos(next_angle), center[1] + next_radius * np.sin(next_angle)

            segments.append(
                Segment(
                    start=(x1, y1),
                    end=(x2, y2),
                    width=trace_width,
                    layer=current_layer,
                )
            )

            # Place thru via at layer change
            if current_layer < layers and turn % layers == 0:
                vias.append(Via(position=(x2, y2), start_layer=0, end_layer=layers - 1))
                current_layer = 0 if current_layer == layers - 1 else current_layer + 1

            current_radius = next_radius
            angle = next_angle

    # Spiral outward (return to near starting point)
    for turn in range(turns // 2, turns):
        while current_radius < max_radius:
            trace_width = min_width + (max_width - min_width) * (current_radius / max_radius) ** width_exponent
            next_radius = current_radius + trace_width + spacing
            next_angle = angle + delta_angle

            x1, y1 = center[0] + current_radius * np.cos(angle), center[1] + current_radius * np.sin(angle)
            x2, y2 = center[0] + next_radius * np.cos(next_angle), center[1] + next_radius * np.sin(next_angle)


            segments.append(
                Segment(
                    start=(x1, y1),
                    end=(x2, y2),
                    width=trace_width,
                    layer=current_layer,
                )
            )

            # Place thru via at layer change
            if current_layer < layers and turn % layers == 0:
                vias.append(Via(position=(x2, y2), start_layer=0, end_layer=layers - 1))
                current_layer = 0 if current_layer == layers - 1 else current_layer + 1

            current_radius = next_radius
            angle = next_angle

    return PCBTrace(layers=layers, segments=segments, vias=vias)
