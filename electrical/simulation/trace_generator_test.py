import pytest
from tvsc.electrical.simulation.trace_generator import generate_spiral_trace
from tvsc.electrical.simulation.pcb_trace import PCBTrace

def test_basic_spiral():
    """Test a simple spiral on a small PCB"""
    center = (0, 0)
    max_radius = 0.045  # 45mm
    turns = 3
    layers = 2
    max_width = 0.5e-3  # 0.5mm
    min_width = 0.2e-3  # 0.2mm
    trace_thickness = 35e-6  # 35µm
    spacing = 0.3e-3  # 0.3mm
    width_exponent = 1.0

    trace = generate_spiral_trace(center, max_radius, turns, layers, max_width, min_width, trace_thickness, spacing, width_exponent)

    assert len(trace.segments) > 0, "Spiral should contain trace segments"
    assert len(trace.vias) > 0, "Vias should be present for layer transitions"
    assert trace.segments[0]['start'] != trace.segments[-1]['end'], "Start and end should be near but not exactly the same"


def test_large_spiral():
    """Test a larger PCB with more layers"""
    center = (0, 0)
    max_radius = 0.05  # 50mm
    turns = 12
    layers = 6
    max_width = 1e-3  # 1mm
    min_width = 0.2e-3  # 0.2mm
    trace_thickness = 35e-6  # 35µm
    spacing = 0.5e-3  # 0.5mm
    width_exponent = 1.5

    trace = generate_spiral_trace(center, max_radius, turns, layers, max_width, min_width, trace_thickness, spacing, width_exponent)

    assert len(trace.segments) > 0, "Should generate segments for a large PCB"
    assert len(trace.vias) > 0, "Should contain vias for multilayer transitions"


def test_varying_width_spiral():
    """Test a spiral with extreme width variation"""
    center = (0, 0)
    max_radius = 0.025  # 25mm
    turns = 7
    layers = 3
    max_width = 1.5e-3  # 1.5mm
    min_width = 0.1e-3  # 100µm
    trace_thickness = 35e-6
    spacing = 0.6e-3
    width_exponent = 2.0  # Large exponent for strong width variation

    trace = generate_spiral_trace(center, max_radius, turns, layers, max_width, min_width, trace_thickness, spacing, width_exponent)

    assert len(trace.segments) > 0, "Segments should be generated"
    assert any(s["width"] > 1e-3 for s in trace.segments), "Some traces should have large width"
    assert any(s["width"] < 0.2e-3 for s in trace.segments), "Some traces should have small width"
