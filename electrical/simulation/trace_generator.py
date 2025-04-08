from itertools import chain
import math
import numpy as np
from typing import List
from .pcb_trace import PCBTrace, TraceSegment, Via

def add_archimedes_spiral(
    trace: PCBTrace,
    start: np.ndarray,
    end: np.ndarray,
    center: np.ndarray,
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
    Generates a PCB trace as an Archimedes spiral from start to end around a center point.

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
        curr_point = np.array([x, y])
        if prev_point is not None:
            trace.add_segment(TraceSegment(start=prev_point, end=curr_point, width=trace_width, layer=current_layer, thickness=trace_thickness))
        theta += dtheta
        r += dr
        prev_point = curr_point

    if prev_point is not None:
        trace.add_segment(TraceSegment(start=prev_point, end=end, width=trace_width, layer=current_layer, thickness=trace_thickness))


def place_points_on_circle(
    n: int,
    min_radius: float,
    min_distance: float,
    start_angle: float = 0,
) -> List[np.ndarray]:
    """
    Evenly places `n` points around a circle with at least `min_distance` between each.
    The radius is increased if needed to satisfy the minimum spacing.

    Args:
        n: Number of points to place.
        min_radius: Minimum radius allowed (meters).
        min_distance: Minimum linear distance between adjacent points (meters).

    Returns:
        List of (x, y) coordinates for the points.
    """
    if n >= 2:
        # Calculate the angle between points
        separation_angle = 2 * math.pi / n

        # Compute the smallest radius that gives at least min_distance between points
        required_radius = min_distance / (2 * math.sin(separation_angle / 2))
        radius = max(min_radius, required_radius)
    else:
        separation_angle = 0
        radius = min_radius

    points = []
    for i in range(n):
        theta = start_angle + i * separation_angle
        x = radius * math.cos(theta)
        y = radius * math.sin(theta)
        points.append(np.array([x, y]))

    return points


def interleave(*iters):
    """
    Given two or more iterables, return a list containing
    the elements of the input list interleaved.

    >>> x = [1, 2, 3, 4]
    >>> y = ('a', 'b', 'c', 'd')
    >>> interleave(x, x)
    [1, 1, 2, 2, 3, 3, 4, 4]
    >>> interleave(x, y, x)
    [1, 'a', 1, 2, 'b', 2, 3, 'c', 3, 4, 'd', 4]

    On a list of lists:
    >>> interleave(*[x, x])
    [1, 1, 2, 2, 3, 3, 4, 4]

    Note that inputs of different lengths will cause the
    result to be truncated at the length of the shortest iterable.
    >>> z = [9, 8, 7]
    >>> interleave(x, z)
    [1, 9, 2, 8, 3, 7]

    On single iterable, or nothing:
    >>> interleave(x)
    [1, 2, 3, 4]
    >>> interleave()
    []
    """
    return list(chain(*zip(*iters)))


def generate_spiral_trace(
    center: np.ndarray,
    radius: float,
    layers: int,
    pad_angle: float,
    max_trace_width: float,
    min_trace_width: float,
    trace_width_exponent: float = 1.0,
    trace_spacing: float = 0.2e-3,
    trace_thickness_outer_layers: float = 35e-6,
    trace_thickness_inner_layers: float = 18e-6,
    via_size: float = 0.000604,
    via_drill_size: float = 0.00035,
    pad_to_track_clearance: float = 0.00015,
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
    layers = 2 * (layers // 2)
    trace = PCBTrace(layers=layers)

    distance_btw_via_centers = via_size + pad_to_track_clearance
    footprint_pad_angle_offset = 2 * math.asin(distance_btw_via_centers / radius / 2)

    inner_via_points = place_points_on_circle(layers // 2, 0, distance_btw_via_centers, pad_angle)
    outer_via_points = place_points_on_circle(layers // 2, radius, distance_btw_via_centers, pad_angle)
    outer_via_points[0] = np.array([radius * math.cos(pad_angle - footprint_pad_angle_offset),
                                    radius * math.sin(pad_angle - footprint_pad_angle_offset)])

    # Reverse the outer via points list so that the via from the back to the front, the last via
    # used, is the one at pad_angle.
    outer_via_points.reverse()

    # Interleave the via points.
    all_points = interleave(inner_via_points, outer_via_points)

    # Generate the vias before we add the starting point. The starting point doesn't have a via.
    trace.vias = list(Via(position=point, size=via_size, drill_size=via_drill_size) for point in all_points)

    # Prepend the starting point.
    all_points.insert(0, np.array([radius * math.cos(pad_angle + footprint_pad_angle_offset),
                                   radius * math.sin(pad_angle + footprint_pad_angle_offset)]))

    # Generate spirals for each layer
    for layer in range(layers):
        start_point = all_points[layer]
        end_point = all_points[layer + 1]

        chirality = "ccw" if layer % 2 == 0 else "cw"

        if layer == 0 or layer == layers - 1:
            trace_thickness = trace_thickness_outer_layers
        else:
            trace_thickness = trace_thickness_inner_layers

        add_archimedes_spiral(
            trace=trace,
            start=start_point,
            end=end_point,
            center=center,
            trace_spacing=trace_spacing,
            max_trace_width=max_trace_width,
            min_trace_width=min_trace_width,
            trace_width_exponent=trace_width_exponent,
            trace_thickness=trace_thickness,
            current_layer=layer,
            chirality=chirality,
            angle_step=angle_step,
        )

    return trace
