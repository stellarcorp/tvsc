import matplotlib.pyplot as plt
import matplotlib.cm as cm
import numpy as np
from .pcb_trace import PCBTrace


def visualize_pcb_trace(trace: PCBTrace, title="PCB Trace Visualization"):
    """
    Visualize a PCBTrace with matplotlib.
    
    Args:
        trace: An instance of PCBTrace.
        title: Plot title.
    """
    if not trace.segments:
        print("No segments to visualize.")
        return

    # Get all layers used
    layers = sorted(set(seg.layer for seg in trace.segments))
    n_layers = max(layers) + 1 if layers else 1

    # Assign each layer a color from a colormap
    colormap = cm.get_cmap('tab10', n_layers)
    layer_colors = {layer: colormap(layer) for layer in range(n_layers)}

    fig, ax = plt.subplots(figsize=(8, 8))
    
    # Draw all segments
    for seg in trace.segments:
        x0, y0 = seg.start
        x1, y1 = seg.end
        ax.plot([x0, x1], [y0, y1], color=layer_colors[seg.layer], linewidth=1.5, label=f"Layer {seg.layer}")

    # Draw vias
    for via in trace.vias:
        ax.plot(via.position[0], via.position[1], 'ko', markersize=3)

    # Make legend unique
    handles, labels = ax.get_legend_handles_labels()
    seen = set()
    unique = [(h, l) for h, l in zip(handles, labels) if not (l in seen or seen.add(l))]
    ax.legend(*zip(*unique), loc='upper right')

    ax.set_title(title)
    ax.set_xlabel("X (m)")
    ax.set_ylabel("Y (m)")
    ax.set_aspect('equal')
    ax.grid(True)
    plt.tight_layout()
    plt.show()
