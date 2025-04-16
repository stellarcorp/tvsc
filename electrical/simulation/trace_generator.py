from itertools import chain
import math
import numpy as np
from scipy.interpolate import CubicHermiteSpline
from typing import List
from .pcb_trace import Marker, Pad, PCB, PCBTrace, TraceSegment, Via

def project_to_squircle(
        squareness: float,
        r: float,
        theta: float,
        x_scale: float,
        y_scale: float) -> np.ndarray:
    rho = r * math.sqrt(2) / (squareness * abs(math.sin(2 * theta))) * math.sqrt(1 - math.sqrt(1 - (squareness * math.sin(2 * theta)) ** 2))

    x = x_scale * rho * math.cos(theta)
    y = y_scale * rho * math.sin(theta)
    return np.array([x, y])


def convert_to_polar(x: float, y: float):
    r = math.hypot(x, y)
    theta = math.atan2(y, x)
    return r, theta


def project_point_to_squircle(
        squareness: float,
        point: np.ndarray,
        x_scale: float,
        y_scale: float) -> np.ndarray:
    r, theta = convert_to_polar(point[0], point[1])
    return project_to_squircle(squareness, r, theta, x_scale, y_scale)


def add_spline(
        trace: PCBTrace,
        start: np.ndarray,
        end: np.ndarray,
        center: np.ndarray,
        squareness: float,
        trace_spacing: float,
        max_trace_width: float,
        min_trace_width: float,
        trace_width_exponent: float,
        trace_thickness: float,
        current_layer: int,
        chirality: str,  # "cw" or "ccw"
        angle_step: float,
        x_scale: float,
        y_scale: float):
    # Vectors from center to endpoints
    vec_start = start - center
    vec_end = end - center

    # Tangent directions are perpendicular to the radius vector
    if chirality == "ccw":
        tangent_start = np.array([-vec_start[1], vec_start[0]])
        tangent_end = np.array([-vec_end[1], vec_end[0]])
    else:
        tangent_start = np.array([vec_start[1], -vec_start[0]])
        tangent_end = np.array([vec_end[1], -vec_end[0]])

    # Parameterization: t in [0, 1]
    t = np.array([0.0, 1.0])
    x = np.array([start[0], end[0]])
    y = np.array([start[1], end[1]])
    dx = np.array([tangent_start[0], tangent_end[0]])
    dy = np.array([tangent_start[1], tangent_end[1]])

    # Create Hermite spline functions. When evaluated at a point on the t interval above, they
    # give spline coordinates.
    spline_x = CubicHermiteSpline(t, x, dx)
    spline_y = CubicHermiteSpline(t, y, dy)

    # Evaluate the spline functions to generate the points for the curve.
    num_points = abs((math.atan2(end[1], end[0]) - math.atan2(start[1], start[0])) / angle_step)
    t_dense = np.linspace(0, 1, num_points)
    curve_x = spline_x(t_dense)
    curve_y = spline_y(t_dense)
    curve_points = np.stack([curve_x, curve_y], axis=-1)

    max_dist = max(np.linalg.norm(start - center), np.linalg.norm(end - center))

    # Build variable-width trace segments
    for i in range(len(curve_points) - 1):
        # Normalize distance for width scaling
        dist = np.linalg.norm(curve_points[i] - center)
        normalized_dist = dist / max_dist if max_dist != 0 else 0
        width = min_trace_width + (max_trace_width - min_trace_width) * (normalized_dist ** trace_width_exponent)

        p0 = project_point_to_squircle(squareness, curve_points[i], x_scale, y_scale)
        p1 = project_point_to_squircle(squareness, curve_points[i+1], x_scale, y_scale)
        trace.add_segment(TraceSegment(start=p0, end=p1, width=width, layer=current_layer, thickness=trace_thickness))


def add_squircle_spiral(
    trace: PCBTrace,
    start: np.ndarray,
    end: np.ndarray,
    center: np.ndarray,
    squareness: float,
    trace_spacing: float,
    max_trace_width: float,
    min_trace_width: float,
    trace_width_exponent: float,
    trace_thickness: float,
    current_layer: int,
    chirality: str,  # "cw" or "ccw"
    angle_step: float,
    x_scale: float,
    y_scale: float,
):
    """
    Generates a PCB trace as a squircle spiral from start to end around a center point. This form
    includes a squareness parameter which allows for a parameterized transition between circle and
    square.

    See https://en.wikipedia.org/wiki/Squircle#Fern%C3%A1ndez-Guasti_squircle for more information.

    All linear units are meters. All angular units are radians.

    Parameters:
        trace: PCBTrace to append to.
        start: (x, y, z) start point.
        end: (x, y, z) end point.
        center: (x, y, z) spiral center.
        squareness: parameter for determining how circle-like (squareness = 0) or square-like
            (squareness = 1) the squircle will be.
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
    r0, theta0 = convert_to_polar(dx0, dy0)
    r1, theta1 = convert_to_polar(dx1, dy1)

    # Set direction for the spiral.
    direction = 1 if chirality == "ccw" else -1

    # Set up trace_width to be a constant across the entire spiral.
    # TODO(james): Modify this code to support a trace_width that changes according to the radius.
    trace_width = min_trace_width

    # Calculate total revolutions (ensure spiral, not arc)
    radial_diff = abs(r1 - r0)
    num_turns = max(1, int(radial_diff / (trace_spacing + trace_width)))

    # Compute total angle to sweep
    angular_difference = direction * (theta1 - theta0)
    while angular_difference < 0:
        angular_difference += 2 * math.pi
    total_angle = 2 * math.pi * num_turns - angular_difference

    # Steps and increments
    steps = max(1, int(abs(total_angle / angle_step)))
    dtheta = total_angle / steps
    dr = (r1 - r0) / steps

    # Generate line segments
    prev_point = None
    curr_point = None
    r = r0
    theta = theta0
    for _ in range(steps):
        curr_point = project_to_squircle(squareness, r, theta, x_scale, y_scale)
        if prev_point is not None:
            trace.add_segment(TraceSegment(start=prev_point, end=curr_point, width=trace_width, layer=current_layer, thickness=trace_thickness))
        theta += dtheta
        r += dr
        prev_point = curr_point

    if prev_point is not None:
        # Build a segment to the end.
        curr_point = project_to_squircle(squareness, r1, theta1, x_scale, y_scale)
        trace.add_segment(TraceSegment(start=prev_point, end=curr_point, width=trace_width, layer=current_layer, thickness=trace_thickness))


def place_points_on_circle(
    n: int,
    min_radius: float,
    min_distance: float,
    start_angle: float = 0
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
    pcb: PCB,
    squareness: float,
    x_radius: float,
    y_radius: float,
    min_radius: float,
    pad_angle: float,
    trace_width_exponent: float = 1.0,
    angle_step: float = 0.05,
    center: np.ndarray = np.array([0.0, 0.0]),
):
    """
    Generate a PCBTrace that spirals inward and outward over multiple layers.

    The spiral starts at the outer radius and spirals inward, switching layers via
    thru vias and spiraling back outward, continuing the pattern until all layers are used.

    Parameters:
        center: (x, y) center of the spiral.
        x_radius: maximum radius of the spiral in the x-direction from center.
        y_radius: maximum radius of the spiral in the y-direction from center.
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
    layers = 2 * (pcb.layers // 2)
    trace = PCBTrace()

    radius = min(x_radius, y_radius)
    x_scale = x_radius / radius
    y_scale = y_radius / radius

    distance_btw_via_centers = pcb.constraints.min_via_diameter + pcb.constraints.trace_spacing
    footprint_pad_angle_offset = 2 * math.asin(distance_btw_via_centers / radius / 2)

    inner_via_points = place_points_on_circle(layers // 2, min_radius, distance_btw_via_centers, pad_angle)
    inner_touch_points =  place_points_on_circle(layers // 2, min_radius + distance_btw_via_centers, distance_btw_via_centers, pad_angle)
    outer_via_points = place_points_on_circle(layers // 2, radius, distance_btw_via_centers, pad_angle)
    outer_touch_points = place_points_on_circle(layers // 2, radius - distance_btw_via_centers, distance_btw_via_centers, pad_angle)

    # Move the pad at the end of the trace so that the start and end do not overlap.
    outer_via_points[0] = np.array([radius * math.cos(pad_angle - footprint_pad_angle_offset),
                                    radius * math.sin(pad_angle - footprint_pad_angle_offset)])

    # Reverse the outer points lists so that the vias are ordered from end to start. The last via
    # is the one at pad_angle.
    outer_via_points.reverse()
    outer_touch_points.reverse()

    # Generate the vias before we prepend the starting point to the list. The starting point doesn't have a via.
    for point in inner_via_points:
        position = project_point_to_squircle(squareness, point, x_scale, y_scale)
        via = Via(position=position, size=pcb.constraints.min_via_diameter, drill_size=pcb.constraints.min_via_drill_size)
        trace.add_via(via)

    for point in outer_via_points:
        position = project_point_to_squircle(squareness, point, x_scale, y_scale)
        via = Via(position=position, size=pcb.constraints.min_via_diameter, drill_size=pcb.constraints.min_via_drill_size)
        trace.add_via(via)

    # Prepend the starting point. It's not truly a via, but like the vias, it is the start (or end) of a single spiral.
    outer_via_points.insert(0, np.array([radius * math.cos(pad_angle + footprint_pad_angle_offset),
                                   radius * math.sin(pad_angle + footprint_pad_angle_offset)]))
    # Also prepend it to the outer_touch_points.
    outer_touch_points.insert(0, outer_via_points[0])

    for touch_point in inner_touch_points:
        position = project_point_to_squircle(squareness, touch_point, x_scale, y_scale)
        trace.add_marker(Marker(position=position, radius=0.0005))

    for touch_point in outer_touch_points:
        position = project_point_to_squircle(squareness, touch_point, x_scale, y_scale)
        trace.add_marker(Marker(position=position, radius=0.0005))

    pcb.add_pad(Pad(1, project_point_to_squircle(squareness, outer_via_points[0], x_scale, y_scale), 0.0015, 0.0025))
    pcb.add_pad(Pad(2, project_point_to_squircle(squareness, outer_via_points[-1], x_scale, y_scale), 0.0015, 0.0025))

    # Generate spirals for each layer. The spiral starts with a spline from the start via to the
    # start touch point. Then, it is a squircle spiral from the start touch point to the end touch
    # point. Then it is a final spline from the end touch point to the end via point.

    # We will flip these list at each layer to make sure that the segments are generated in a
    # direction consistent with the current.
    start_via_list = outer_via_points
    start_touch_point_list = outer_touch_points
    end_via_list = inner_via_points
    end_touch_point_list = inner_touch_points

    start_via_index = 0
    end_via_index = 0
    for layer in range(layers):
        start_touch_point_index = (start_via_index + 1) % len(start_touch_point_list)
        end_touch_point_index = end_via_index - 1

        chirality = "ccw" if layer % 2 == 0 else "cw"

        if layer == 0 or layer == pcb.layers - 1:
            trace_thickness = pcb.constraints.trace_thickness_outer_layers
        else:
            trace_thickness = pcb.constraints.trace_thickness_inner_layers

        add_squircle_spiral(
            trace=trace,
            start=start_via_list[start_via_index],
            end=start_touch_point_list[start_touch_point_index],
            center=center,
            squareness=squareness,
            trace_spacing=pcb.constraints.trace_spacing,
            max_trace_width=pcb.constraints.max_trace_width,
            min_trace_width=pcb.constraints.min_trace_width,
            trace_width_exponent=trace_width_exponent,
            trace_thickness=trace_thickness,
            current_layer=layer,
            chirality=chirality,
            angle_step=angle_step,
            x_scale=x_scale,
            y_scale=y_scale,
        )

        add_squircle_spiral(
            trace=trace,
            start=start_touch_point_list[start_touch_point_index],
            end=end_touch_point_list[end_touch_point_index],
            center=center,
            squareness=squareness,
            trace_spacing=pcb.constraints.trace_spacing,
            max_trace_width=pcb.constraints.max_trace_width,
            min_trace_width=pcb.constraints.min_trace_width,
            trace_width_exponent=trace_width_exponent,
            trace_thickness=trace_thickness,
            current_layer=layer,
            chirality=chirality,
            angle_step=angle_step,
            x_scale=x_scale,
            y_scale=y_scale,
        )

        add_squircle_spiral(
            trace=trace,
            start=end_touch_point_list[end_touch_point_index],
            end=end_via_list[end_via_index],
            center=center,
            squareness=squareness,
            trace_spacing=pcb.constraints.trace_spacing,
            max_trace_width=pcb.constraints.max_trace_width,
            min_trace_width=pcb.constraints.min_trace_width,
            trace_width_exponent=trace_width_exponent,
            trace_thickness=trace_thickness,
            current_layer=layer,
            chirality=chirality,
            angle_step=angle_step,
            x_scale=x_scale,
            y_scale=y_scale,
        )

        start_via_list, end_via_list = end_via_list, start_via_list
        start_touch_point_list, end_touch_point_list = end_touch_point_list, start_touch_point_list
        start_via_index, end_via_index = end_via_index, start_via_index
        end_via_index += 1

    pcb.add_trace(trace)
