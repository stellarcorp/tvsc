from collections import defaultdict
import matplotlib.pyplot as plt
import matplotlib.cm as cm
from matplotlib.widgets import CheckButtons
import numpy as np
from .pcb_trace import PCBTrace


def visualize_pcb(pcb, figsize=(10, 10)):
    """
    Visualize a multilayer PCB with interactive layer toggles and zoom.
    Vias are shown as vertical connections between layers.

    Args:
        pcb: PCB instance from tvsc.electrical.simulation.pcb_trace.
        figsize: Size of the matplotlib figure.
    """
    fig, ax = plt.subplots(figsize=figsize)
    plt.subplots_adjust(left=0.25)

    # Organize traces by layer
    layers = defaultdict(list)
    for net in pcb.nets:
        for trace in net.traces:
            layers[trace.layer].append(trace)

    # Plot each layer's traces
    layer_lines = {}
    colormap = cm.get_cmap('prism', len(layers))
    layer_colors = {layer: colormap(layer) for layer in range(len(layers))}
    layer_order = sorted(layers.keys())

    for i, layer in enumerate(layer_order):
        xs, ys = [], []
        for trace in layers[layer]:
            for seg in trace.segments:
                xs.extend([seg.start[0], seg.end[0], None])
                ys.extend([seg.start[1], seg.end[1], None])
        line, = ax.plot(xs, ys, label=f"Layer {layer}", color=layer_colors[layer])
        layer_lines[layer] = line

    # Draw markers
    for marker in pcb.markers:
        ax.plot(marker.position[0], marker.position[1], 'rx', markersize=10)  # dot at marker location

    # Draw vias
    for via in pcb.vias:
        ax.plot(via.position[0], via.position[1], 'ko', markersize=3)  # dot at via location

    # Axis settings
    ax.set_title("PCB Visualization")
    ax.set_xlabel("X [m]")
    ax.set_ylabel("Y [m]")
    ax.set_aspect("equal", "box")
    ax.grid(True)

    # Layer toggle checkboxes
    layer_names = [f"Layer {l}" for l in layer_order]
    visibility = [True] * len(layer_order)
    rax = plt.axes([0.02, 0.4, 0.18, 0.4])
    check = CheckButtons(rax, layer_names, visibility)

    def toggle_layer(label):
        idx = layer_names.index(label)
        layer = layer_order[idx]
        line = layer_lines[layer]
        line.set_visible(not line.get_visible())
        plt.draw()

    check.on_clicked(toggle_layer)

    plt.show()
