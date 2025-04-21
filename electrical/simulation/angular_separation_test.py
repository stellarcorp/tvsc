import math
import numpy as np
import pytest
from pytest import approx
from .trace_generator import compute_angular_separation


def test_can_compute_trivial_separations_ccw():
    assert approx(math.pi / 4) == compute_angular_separation(0., math.pi / 4, 'ccw')
    assert approx(math.pi / 2) == compute_angular_separation(0., math.pi / 2, 'ccw')
    assert approx(3 * math.pi / 4) == compute_angular_separation(0., 3 * math.pi / 4, 'ccw')
    assert approx(math.pi) == compute_angular_separation(0., math.pi, 'ccw')
    assert approx(5 * math.pi / 4) == compute_angular_separation(0., 5 * math.pi / 4, 'ccw')
    assert approx(3 * math.pi / 2) == compute_angular_separation(0., 3 * math.pi / 2, 'ccw')
    assert approx(7 * math.pi / 4) == compute_angular_separation(0., 7 * math.pi / 4, 'ccw')
    assert approx(0.) == compute_angular_separation(0., 2 * math.pi, 'ccw')


def test_can_compute_trivial_separations_cw():
    assert approx(math.pi / 4) == compute_angular_separation(math.pi / 4, 0., 'cw')
    assert approx(math.pi / 2) == compute_angular_separation(math.pi / 2, 0., 'cw')
    assert approx(3 * math.pi / 4) == compute_angular_separation(3 * math.pi / 4, 0., 'cw')
    assert approx(math.pi) == compute_angular_separation(math.pi, 0., 'cw')
    assert approx(5 * math.pi / 4) == compute_angular_separation(5 * math.pi / 4, 0., 'cw')
    assert approx(3 * math.pi / 2) == compute_angular_separation(3 * math.pi / 2, 0., 'cw')
    assert approx(7 * math.pi / 4) == compute_angular_separation(7 * math.pi / 4, 0., 'cw')
    assert approx(0.) == compute_angular_separation(2 * math.pi, 0., 'cw')


def test_can_compute_negative_separations_ccw():
    assert approx(2 * math.pi - math.pi / 4) == compute_angular_separation(math.pi / 4, 0., 'ccw')
    assert approx(2 * math.pi - math.pi / 2) == compute_angular_separation(math.pi / 2, 0., 'ccw')
    assert approx(2 * math.pi - 3 * math.pi / 4) == compute_angular_separation(3 * math.pi / 4, 0., 'ccw')
    assert approx(2 * math.pi - math.pi) == compute_angular_separation(math.pi, 0., 'ccw')
    assert approx(2 * math.pi - 5 * math.pi / 4) == compute_angular_separation(5 * math.pi / 4, 0., 'ccw')
    assert approx(2 * math.pi - 3 * math.pi / 2) == compute_angular_separation(3 * math.pi / 2, 0., 'ccw')
    assert approx(2 * math.pi - 7 * math.pi / 4) == compute_angular_separation(7 * math.pi / 4, 0., 'ccw')
    assert approx(0.) == compute_angular_separation(2 * math.pi, 0., 'ccw')


def test_can_compute_negative_separations_cw():
    assert approx(2 * math.pi - math.pi / 4) == compute_angular_separation(0., math.pi / 4, 'cw')
    assert approx(2 * math.pi - math.pi / 2) == compute_angular_separation(0., math.pi / 2, 'cw')
    assert approx(2 * math.pi - 3 * math.pi / 4) == compute_angular_separation(0., 3 * math.pi / 4, 'cw')
    assert approx(2 * math.pi - math.pi) == compute_angular_separation(0., math.pi, 'cw')
    assert approx(2 * math.pi - 5 * math.pi / 4) == compute_angular_separation(0., 5 * math.pi / 4, 'cw')
    assert approx(2 * math.pi - 3 * math.pi / 2) == compute_angular_separation(0., 3 * math.pi / 2, 'cw')
    assert approx(2 * math.pi - 7 * math.pi / 4) == compute_angular_separation(0., 7 * math.pi / 4, 'cw')
    assert approx(0.) == compute_angular_separation(0., 2 * math.pi, 'cw')


def test_can_compute_larger_separations_ccw():
    assert approx(7 * math.pi / 4) == compute_angular_separation(math.pi / 2, math.pi / 4, 'ccw')
    assert approx(0.) == compute_angular_separation(math.pi / 2, math.pi / 2, 'ccw')
    assert approx(math.pi / 4) == compute_angular_separation(math.pi / 2, 3 * math.pi / 4, 'ccw')
    assert approx(math.pi / 2) == compute_angular_separation(math.pi / 2, math.pi, 'ccw')
    assert approx(3 * math.pi / 4) == compute_angular_separation(math.pi / 2, 5 * math.pi / 4, 'ccw')
    assert approx(math.pi) == compute_angular_separation(math.pi / 2, 3 * math.pi / 2, 'ccw')
    assert approx(5 * math.pi / 4) == compute_angular_separation(math.pi / 2, 7 * math.pi / 4, 'ccw')
    assert approx(3 * math.pi / 2) == compute_angular_separation(math.pi / 2, 2 * math.pi, 'ccw')


def test_can_start_with_negative_angles_cw():
    assert approx(5 * math.pi / 4) == compute_angular_separation(-math.pi / 2, math.pi / 4, 'cw')
    assert approx(math.pi) == compute_angular_separation(-math.pi / 2, math.pi / 2, 'cw')
    assert approx(3 * math.pi / 4) == compute_angular_separation(-math.pi / 2, 3 * math.pi / 4, 'cw')
    assert approx(math.pi / 2) == compute_angular_separation(-math.pi / 2, math.pi, 'cw')
    assert approx(math.pi / 4) == compute_angular_separation(-math.pi / 2, 5 * math.pi / 4, 'cw')
    assert approx(0) == compute_angular_separation(-math.pi / 2, 3 * math.pi / 2, 'cw')
    assert approx(7 * math.pi / 4) == compute_angular_separation(-math.pi / 2, 7 * math.pi / 4, 'cw')
    assert approx(3 * math.pi / 2) == compute_angular_separation(-math.pi / 2, 2 * math.pi, 'cw')
