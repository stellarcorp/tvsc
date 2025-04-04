from dataclasses import dataclass
from typing import List, Tuple

@dataclass
class Segment:
    start: Tuple[float, float]
    end: Tuple[float, float]
    width: float
    layer: int

@dataclass
class Via:
    x: float
    y: float
    start_layer: int
    end_layer: int

@dataclass
class PCBTrace:
    segments: List[Segment]
    vias: List[Via]
