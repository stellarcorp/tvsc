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
    thickness: float

    def reverse(self):
        self.start, self.end = self.end, self.start


@dataclass
class Via:
    """
    Represents a thru via in a PCB.
    """
    position: np.ndarray
    size: float
    drill_size: float


@dataclass
class Pad:
    """
    Represents a pad on a PCB footprint.
    """
    index: int
    position: np.ndarray
    x_size: float
    y_size: float


@dataclass
class Marker:
    """
    Represents a marker for debugging or drawing attention to certain areas.
    """
    position: np.ndarray
    radius: float


@dataclass
class PCBTrace:
    """
    Represents a full trace consisting of segments and vias.

    Attributes:
        segments: list of trace segments
        vias: list of vias connecting layers
    """
    layer: int = 0
    segments: List[TraceSegment] = field(default_factory=list)

    def add_segment(self, segment: TraceSegment):
        self.segments.append(segment)

    def reverse(self):
        # Reverse the direction of a trace. Traces have an intrinsic direction that is the
        # direction of current flow under a positive voltage. This method reverses that direction.
        for seg in self.segments:
            seg.reverse()

    def total_length(self) -> float:
        return sum(
            np.linalg.norm(seg.end - seg.start) for seg in self.segments)

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
        """Estimate magnetic moment vector (A·m²)."""
        moment = np.zeros(3)
        for seg in self.segments:
            r = (seg.start + seg.end) / 2  # midpoint of segment
            dl = seg.end - seg.start  # vector of segment
            # Compute the directed area swept out by triangle from zero to start and end.
            area = np.cross(r, dl) / 2
            if np.shape(area) == ():
                area = np.array([0., 0., area])
            moment += current * area
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
class Net:
    """
    Represents a net on a PCB.
    """
    traces: List[PCBTrace] = field(default_factory=list)

    def add_trace(self, trace: PCBTrace):
        self.traces.append(trace)

    def estimate_resistance(self):
        # In reality, computing the resistance of a net is a meaningless concept. At a minimum, we
        # should specify two points to calculate the resistance between. This codes makes a number
        # of assumptions that make the code below a reasonable approach. We assume that the net only
        # has two end points and that we want the resistance between those end points. We also
        # assume that any traces on different layers are connected with vias and that those vias
        # have zero resistance. Further, we assume that all traces in the net are in series.
        resistance = 0
        for trace in self.traces:
            resistance += trace.estimate_resistance()
        return resistance


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
    nets: List[Net] = field(default_factory=list)
    vias: List[Via] = field(default_factory=list)
    markers: List[Marker] = field(default_factory=list)
    pads: List[Pad] = field(default_factory=list)

    def add_net(self, net: Net):
        self.nets.append(net)

    def add_pad(self, pad: Pad):
        self.pads.append(pad)

    def add_via(self, via: Via):
        self.vias.append(via)

    def add_marker(self, marker: Marker):
        self.markers.append(marker)
