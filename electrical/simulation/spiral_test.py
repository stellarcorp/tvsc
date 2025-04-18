import math
import numpy as np
import pytest
from pytest import approx
from .pcb_trace import PCBTrace, TraceSegment
from .trace_generator import create_squircle_spiral


def test_can_create_trivial_arcs_ccw_horizontal_end():
    # Create a trivial arcs from (1, 0) to different end points, centered on (0, 0). These do not have any concentric curves.
    start = np.array([1., 0.])

    for end in (np.array([0., -1.]), np.array([0., 1.])):
        trace = create_squircle_spiral(start=start,
                                       end=end,
                                       center=np.array([0., 0.]),
                                       squareness=0.,
                                       trace_spacing=0.5,
                                       max_trace_width=0.5,
                                       min_trace_width=0.5,
                                       trace_width_exponent=0.,
                                       trace_thickness=0.5,
                                       chirality="ccw",
                                       angle_step=0.1,
                                       x_scale=1.,
                                       y_scale=1.)

        assert trace.segments[0].start == approx(
            start), "Arc should start at the start point"
        assert trace.segments[-1].end == approx(
            end), "Arc should end at the end point"

        inverse_slope = (trace.segments[0].end[0] - trace.segments[0].start[0]
                         ) / (trace.segments[0].end[1] -
                              trace.segments[0].start[1])
        assert inverse_slope < 0 and abs(
            inverse_slope
        ) < 0.1, "Inverse of slope of first segment should be near zero and negative."

        assert np.linalg.norm(trace.segments[-1].start - end) < np.linalg.norm(
            start - end
        ), f"Start of the last segment ({trace.segments[-1].start[0]}, {trace.segments[-1].start[1]}) should be closer to the ending point ({end[0]}, {end[1]}) than the starting point ({start[0]}, {start[1]}) is."

        slope = (trace.segments[-1].end[1] - trace.segments[-1].start[1]) / (
            trace.segments[-1].end[0] - trace.segments[-1].start[0])
        assert abs(
            slope
        ) < 0.1, f"Slope of last segment should be near zero. ({trace.segments[-1].start[0]}, {trace.segments[-1].start[1]})->({trace.segments[-1].end[0]}, {trace.segments[-1].end[1]})"


def test_can_create_trivial_arcs_ccw_vertical_end():
    # Create a trivial arcs from (1, 0) to different end points, centered on (0, 0). These do not have any concentric curves.
    start = np.array([1., 0.])

    for end in (np.array([1., 0.]), np.array([-1., 0.])):
        trace = create_squircle_spiral(start=start,
                                       end=end,
                                       center=np.array([0., 0.]),
                                       squareness=0.,
                                       trace_spacing=0.5,
                                       max_trace_width=0.5,
                                       min_trace_width=0.5,
                                       trace_width_exponent=0.,
                                       trace_thickness=0.5,
                                       chirality="ccw",
                                       angle_step=0.1,
                                       x_scale=1.,
                                       y_scale=1.)

        assert trace.segments[0].start == approx(
            start), "Arc should start at the start point"
        assert trace.segments[-1].end == approx(
            end), "Arc should end at the end point"

        inverse_slope = (trace.segments[0].end[0] - trace.segments[0].start[0]
                         ) / (trace.segments[0].end[1] -
                              trace.segments[0].start[1])
        assert inverse_slope < 0 and abs(
            inverse_slope
        ) < 0.1, "Inverse of slope of first segment should be near zero and negative."

        if not np.array_equal(start, end):
            assert np.linalg.norm(
                trace.segments[-1].start - end
            ) < np.linalg.norm(
                start - end
            ), f"Start of the last segment ({trace.segments[-1].start[0]}, {trace.segments[-1].start[1]}) should be closer to the ending point ({end[0]}, {end[1]}) than the starting point ({start[0]}, {start[1]}) is."

        inverse_slope = (
            trace.segments[-1].end[0] - trace.segments[-1].start[0]) / (
                trace.segments[-1].end[1] - trace.segments[-1].start[1])
        assert abs(inverse_slope
                   ) < 0.1, "Inverse slope of last segment should be near zero"


def test_can_create_trivial_arcs_cw_horizontal_end():
    # Create a trivial arcs from (1, 0) to different end points, centered on (0, 0). These do not have any concentric curves.
    start = np.array([1., 0.])

    for end in (np.array([0., -1.]), np.array([0., 1.])):
        trace = create_squircle_spiral(start=start,
                                       end=end,
                                       center=np.array([0., 0.]),
                                       squareness=0.,
                                       trace_spacing=0.5,
                                       max_trace_width=0.5,
                                       min_trace_width=0.5,
                                       trace_width_exponent=0.,
                                       trace_thickness=0.5,
                                       chirality="cw",
                                       angle_step=0.1,
                                       x_scale=1.,
                                       y_scale=1.)

        assert trace.segments[0].start == approx(
            start), "Arc should start at the start point"
        assert trace.segments[-1].end == approx(
            end), "Arc should end at the end point"

        inverse_slope = (trace.segments[0].end[0] - trace.segments[0].start[0]
                         ) / (trace.segments[0].end[1] -
                              trace.segments[0].start[1])
        assert inverse_slope < 0 and abs(
            inverse_slope
        ) < 0.1, "Inverse of slope of first segment should be near zero and negative."

        assert np.linalg.norm(trace.segments[-1].start - end) < np.linalg.norm(
            start - end
        ), f"Start of the last segment ({trace.segments[-1].start[0]}, {trace.segments[-1].start[1]}) should be closer to the ending point ({end[0]}, {end[1]}) than the starting point ({start[0]}, {start[1]}) is."

        slope = (trace.segments[-1].end[1] - trace.segments[-1].start[1]) / (
            trace.segments[-1].end[0] - trace.segments[-1].start[0])
        assert abs(
            slope
        ) < 0.1, f"Slope of last segment should be near zero. ({trace.segments[-1].start[0]}, {trace.segments[-1].start[1]})->({trace.segments[-1].end[0]}, {trace.segments[-1].end[1]})"


def test_can_create_trivial_arcs_cw_vertical_end():
    # Create a trivial arcs from (1, 0) to different end points, centered on (0, 0). These do not have any concentric curves.
    start = np.array([1., 0.])

    for end in (np.array([1., 0.]), np.array([-1., 0.])):
        trace = create_squircle_spiral(start=start,
                                       end=end,
                                       center=np.array([0., 0.]),
                                       squareness=0.,
                                       trace_spacing=0.5,
                                       max_trace_width=0.5,
                                       min_trace_width=0.5,
                                       trace_width_exponent=0.,
                                       trace_thickness=0.5,
                                       chirality="cw",
                                       angle_step=0.1,
                                       x_scale=1.,
                                       y_scale=1.)

        assert trace.segments[0].start == approx(
            start), "Arc should start at the start point"
        assert trace.segments[-1].end == approx(
            end), "Arc should end at the end point"

        inverse_slope = (trace.segments[0].end[0] - trace.segments[0].start[0]
                         ) / (trace.segments[0].end[1] -
                              trace.segments[0].start[1])
        assert inverse_slope < 0 and abs(
            inverse_slope
        ) < 0.1, "Inverse of slope of first segment should be near zero and negative."

        if not np.array_equal(start, end):
            assert np.linalg.norm(
                trace.segments[-1].start - end
            ) < np.linalg.norm(
                start - end
            ), f"Start of the last segment ({trace.segments[-1].start[0]}, {trace.segments[-1].start[1]}) should be closer to the ending point ({end[0]}, {end[1]}) than the starting point ({start[0]}, {start[1]}) is."

        inverse_slope = (
            trace.segments[-1].end[0] - trace.segments[-1].start[0]) / (
                trace.segments[-1].end[1] - trace.segments[-1].start[1])
        assert abs(inverse_slope
                   ) < 0.1, "Inverse slope of last segment should be near zero"
