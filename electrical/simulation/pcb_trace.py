from dataclasses import dataclass, field
from typing import List, Tuple


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
    start: Tuple[float, float]
    end: Tuple[float, float]
    width: float
    layer: int
    thickness: float


@dataclass
class Via:
    """
    Represents a via connecting two layers.

    Attributes:
        position: (x, y) coordinate in meters
        start_layer: starting layer index (0-based)
        end_layer: ending layer index (0-based)
    """
    position: Tuple[float, float]
    start_layer: int
    end_layer: int


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

    def add_via(self, via: Via):
        self.vias.append(via)

    def total_length(self) -> float:
        return sum(
            ((seg.end[0] - seg.start[0]) ** 2 + (seg.end[1] - seg.start[1]) ** 2) ** 0.5
            for seg in self.segments
        )

    def total_resistance(self, resistivity: float = 1.68e-8) -> float:
        """
        Compute the total resistance of the trace.

        Parameters:
            resistivity: resistivity of trace material in ohm-meters (default is copper)

        Returns:
            Total resistance in ohms.
        """
        resistance = 0.0
        for seg in self.segments:
            length = ((seg.end[0] - seg.start[0]) ** 2 + (seg.end[1] - seg.start[1]) ** 2) ** 0.5
            area = seg.width * seg.thickness
            if area > 0:
                resistance += resistivity * length / area
        return resistance
