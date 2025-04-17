import math
import numpy as np
import pytest
from pytest import approx
from .pcb_trace import PCBTrace, TraceSegment
from .trace_generator import create_squircle_spiral


def test_can_create_trivial_arcs_ccw():
    # Create a trivial arcs from (1, 0) to various end points, centered on (0, 0). These do not have any concentric curves.
    start=np.array([1., 0.])

    for end in (np.array([0., 1.]), np.array([-1., 0.]), np.array([0., -1.])):
        trace = create_squircle_spiral(
            start=start,
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

        assert trace.segments[0].start == approx(start), "Arc should start at the start point"
        assert trace.segments[-1].end == approx(end), "Arc should end at the end point"


def test_can_create_trivial_arcs_cw():
    # Create a trivial arcs from (1, 0) to various end points, centered on (0, 0). These do not have any concentric curves.
    start=np.array([1., 0.])

    for end in (np.array([0., 1.]), np.array([-1., 0.]), np.array([0., -1.])):
        trace = create_squircle_spiral(
            start=start,
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

        assert trace.segments[0].start == approx(start), "Arc should start at the start point"
        assert trace.segments[-1].end == approx(end), "Arc should end at the end point"
