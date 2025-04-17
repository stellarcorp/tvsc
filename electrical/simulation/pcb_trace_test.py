import math
import numpy as np
import pytest
from pytest import approx
from .pcb_trace import PCBTrace, TraceSegment


def test_can_compute_length():
    trace = PCBTrace()

    segment1 = TraceSegment(np.array([0., 0., 0.]), np.array([0., 1., 0.]), 1., 1.)
    trace.add_segment(segment1)
    assert trace.total_length() == approx(1.), "Length computation incorrect"

    segment2 = TraceSegment(np.array([0., 1., 0.]), np.array([0., 2., 0.]), 1., 1.)
    trace.add_segment(segment2)
    assert trace.total_length() == approx(2.), "Length computation incorrect"


def test_length_computation_follows_short_path():
    trace = PCBTrace()

    segment1 = TraceSegment(np.array([0., 0., 0.]), np.array([0., 1., 0.]), 1., 1.)
    trace.add_segment(segment1)
    assert trace.total_length() == approx(1.), "Length computation incorrect"

    segment2 = TraceSegment(np.array([0., 1., 0.]), np.array([1., 1., 0.]), 1., 1.)
    trace.add_segment(segment2)
    assert trace.total_length() == approx(2), "Length computation incorrect"


def test_length_computation_follows_path():
    trace = PCBTrace()

    segment1 = TraceSegment(np.array([0., 0., 0.]), np.array([0., 1., 0.]), 1., 1.)
    trace.add_segment(segment1)
    segment2 = TraceSegment(np.array([0., 1., 0.]), np.array([1., 1., 0.]), 1., 1.)
    trace.add_segment(segment2)
    segment3 = TraceSegment(np.array([1., 1., 0.]), np.array([1., 0., 0.]), 1., 1.)
    trace.add_segment(segment3)
    segment4 = TraceSegment(np.array([1., 0., 0.]), np.array([0., 0., 0.]), 1., 1.)
    trace.add_segment(segment4)
    assert trace.total_length() == approx(4), "Length computation incorrect"


def test_wire_resistance_non_zero():
    trace = PCBTrace()

    segment = TraceSegment(np.array([0., 0., 0.]), np.array([1., 1., 0.]), 1., 1.)
    trace.add_segment(segment)

    assert trace.estimate_resistance() > 0, "Wire should have some resistance"


def test_wire_resistance_doubles_when_length_doubles():
    trace1 = PCBTrace()
    trace2 = PCBTrace()

    segment1 = TraceSegment(np.array([0., 0., 0.]), np.array([0., 1., 0.]), 1., 1.)
    trace1.add_segment(segment1)
    resistance1 = trace1.estimate_resistance()

    segment2 = TraceSegment(np.array([0., 0., 0.]), np.array([0., 2., 0.]), 1., 1.)
    trace2.add_segment(segment2)
    resistance2 = trace2.estimate_resistance()

    assert 2 * resistance1 == approx(resistance2), "Doubling length should double resistance"


def test_wire_resistance_halves_when_width_doubles():
    trace1 = PCBTrace()
    trace2 = PCBTrace()

    segment1 = TraceSegment(np.array([0., 0., 0.]), np.array([0., 1., 0.]), 1., 1.)
    trace1.add_segment(segment1)
    resistance1 = trace1.estimate_resistance()

    segment2 = TraceSegment(np.array([0., 0., 0.]), np.array([0., 1., 0.]), 2., 1.)
    trace2.add_segment(segment2)
    resistance2 = trace2.estimate_resistance()

    assert 0.5 * resistance1 == approx(resistance2), "Doubling width should halve resistance"


def test_wire_resistance_halves_when_thickness_doubles():
    trace1 = PCBTrace()
    trace2 = PCBTrace()

    segment1 = TraceSegment(np.array([0., 0., 0.]), np.array([0., 1., 0.]), 1., 1.)
    trace1.add_segment(segment1)
    resistance1 = trace1.estimate_resistance()

    segment2 = TraceSegment(np.array([0., 0., 0.]), np.array([0., 1., 0.]), 1., 2.)
    trace2.add_segment(segment2)
    resistance2 = trace2.estimate_resistance()

    assert 0.5 * resistance1 == approx(resistance2), "Doubling thickness should halve resistance"


def test_magnetic_moment_is_vector():
    trace = PCBTrace()

    segment1 = TraceSegment(np.array([0., 0., 0.]), np.array([0., 1., 0.]), 1., 1.)
    trace.add_segment(segment1)
    assert np.size(trace.estimate_magnetic_moment(1.)) == np.array(3), "Magnetic moment not a vector"


def test_no_magnetic_moment_for_single_trace_segment():
    trace = PCBTrace()

    segment1 = TraceSegment(np.array([0., 0., 0.]), np.array([0., 1., 0.]), 1., 1.)
    trace.add_segment(segment1)
    assert np.linalg.norm(trace.estimate_magnetic_moment(1.)) == approx(0.), "Magnetic moment computation incorrect"


def test_magnetic_moment_computation_of_square():
    trace = PCBTrace()

    segment1 = TraceSegment(np.array([0., 0., 0.]), np.array([0., 1., 0.]), 1., 1.)
    trace.add_segment(segment1)
    segment2 = TraceSegment(np.array([0., 1., 0.]), np.array([1., 1., 0.]), 1., 1.)
    trace.add_segment(segment2)
    segment3 = TraceSegment(np.array([1., 1., 0.]), np.array([1., 0., 0.]), 1., 1.)
    trace.add_segment(segment3)
    segment4 = TraceSegment(np.array([1., 0., 0.]), np.array([0., 0., 0.]), 1., 1.)
    trace.add_segment(segment4)
    # Magnetic moment of a wire in the shape of a square is the area of the square times the current.
    # The area here is 1, but with a left-handed orientation. The current is 3.
    assert trace.estimate_magnetic_moment(3.) == approx(np.array([0., 0., -3.])), "Magnetic moment computation incorrect"


def test_2d_points_augmented_to_3d():
    # Here we set up a magnetic moment calculation, but specify all of the start and end points in
    # 2D, rather than 3D.
    # The TraceSegment should extend those points to 3D so that the cross products used in the
    # magnetic moment calculation are valid.
    trace = PCBTrace()

    segment1 = TraceSegment(np.array([0., 0.]), np.array([0., 1.]), 1., 1.)
    trace.add_segment(segment1)
    segment2 = TraceSegment(np.array([0., 1.]), np.array([1., 1.]), 1., 1.)
    trace.add_segment(segment2)
    segment3 = TraceSegment(np.array([1., 1.]), np.array([1., 0.]), 1., 1.)
    trace.add_segment(segment3)
    segment4 = TraceSegment(np.array([1., 0.]), np.array([0., 0.]), 1., 1.)
    trace.add_segment(segment4)
    # Magnetic moment of a wire in the shape of a square is the area of the square times the current.
    # The area here is 1, but with a left-handed orientation. The current is 3.
    assert trace.estimate_magnetic_moment(3.) == approx(np.array([0., 0., -3.])), "Magnetic moment computation incorrect"
