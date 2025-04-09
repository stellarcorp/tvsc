import numpy as np
from dataclasses import dataclass, field
from typing import List

@dataclass
class TraceSegment:
    """
    Represents a segment of a PCB trace.

    Attributes:
        start: (x, y) coordinate in meters
        end: (x, y) coordinate in meters
        width: trace width in meters
        layer: integer index of the PCB layer (0-based)
        thickness: copper thickness in meters
    """
    start: np.ndarray
    end: np.ndarray
    width: float
    layer: int
    thickness: float


@dataclass
class Via:
    """
    Represents a thru via in a PCB.

    Attributes:
        position: (x, y) coordinate in meters
    """
    position: np.ndarray
    size: float
    drill_size: float


@dataclass
class PCBTrace:
    """
    Represents a full trace consisting of segments and vias.

    Attributes:
        segments: list of trace segments
        vias: list of vias connecting layers
    """
    segments: List[TraceSegment] = field(default_factory=list)
    vias: List[Via] = field(default_factory=list)
    layers: int = field(default_factory=int)

    def add_segment(self, segment: TraceSegment):
        self.segments.append(segment)

    def concatenate(self, trace):
        self.segments.append(trace.segments)
        self.vias.append(trace.vias)

    def add_via(self, via: Via):
        self.vias.append(via)

    def total_length(self) -> float:
        return sum(np.linalg.norm(seg.end - seg.start)
            for seg in self.segments
        )

    def estimate_resistance(self, resistivity: float = 1.68e-8) -> float:
        """
        Estimate the total resistance of the trace.

        Parameters:
            resistivity: resistivity of trace material in ohm-meters (default is copper)

        Returns:
            Total resistance in ohms.
        """
        resistance = 0.0
        for seg in self.segments:
            length = np.linalg.norm(seg.end - seg.start)
            area = seg.width * seg.thickness
            if area > 0:
                resistance += resistivity * length / area
        return resistance

    def estimate_magnetic_moment(self, current: float) -> np.ndarray:
        """Estimate magnetic moment vector (A·m²). Assumes loops lie in XY plane."""
        moment = np.zeros(3)
        for seg in self.segments:
            r = (seg.start + seg.end) / 2  # midpoint of segment
            dl = seg.end - seg.start       # vector of segment
            area_vec = np.cross(np.append(r, 0), np.append(dl, 0))
            moment += current * area_vec / 2
        return moment


@dataclass
class PCBConstraints:
    """
    Represents the manufacturability / cost constraints on a PCB.
    """
    max_trace_width: float = 0
    min_trace_width: float = 0
    trace_thickness_inner_layers: float = 0
    trace_thickness_inner_layers: float = 0
    trace_spacing: float = 0
    min_via_drill_size: float = 0
    min_via_diameter: float = 0


@dataclass
class PCB:
    """
    Represents a Printed Circuit Board.

    Attributes:
        size: size, including thickness, of the PCB in meters
        layers: number of layers in the PCB
        constraints: manufacturing constraints that need to be met
    """
    size: np.ndarray = np.array([0, 0])
    layers: int = 0
    constraints: PCBConstraints = PCBConstraints()
    traces: List[PCBTrace] = field(default_factory=list)

    def add_trace(self, trace: PCBTrace):
        self.traces.append(trace)
