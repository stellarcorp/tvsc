def get_kicad_layer(layer_index: int, total_layers: int) -> str:
    """
    Maps a numeric layer index to a KiCad layer name.
    Supports up to 32 layers.
    """
    if total_layers < 1 or total_layers > 32:
        raise ValueError("Only 1–32 layers supported in KiCad.")

    if layer_index == 0:
        return "F.Cu"
    elif layer_index == total_layers - 1:
        return "B.Cu"
    elif 1 <= layer_index < total_layers - 1:
        return f"In{layer_index}.Cu"
    else:
        raise ValueError(f"Invalid layer index {layer_index} for {total_layers} layers.")


def generate_kicad_footprint(pcb, filename: str, footprint_name: str = "Magnetorquer"):
    """
    Generates a KiCad 6+ compatible footprint (.kicad_mod) for the given PCBTrace.
    """
    from datetime import datetime

    def format_coord(x):
        return f"{x * 1000:.3f}"  # Convert meters to mm

    total_layers = pcb.layers

    lines = [
        f"(module {footprint_name} (layer F.Cu) (tedit {datetime.now().strftime('%Y%m%d')}00)",
        "  (attr through_hole allow_missing_courtyard)",
        "  (fp_text reference REF** (at 0 0) (layer F.Fab)",
        "    (effects (font (size 1 1) (thickness 0.15)))",
        "  )",
        "  (fp_text value {} (at 0 -1.5) (layer F.Fab)".format(footprint_name),
        "    (effects (font (size 1 1) (thickness 0.15)))",
        "  )",
        "  (fp_circle (center 0 0) (end 1 0)",
        "    (stroke (width 0.1) (type default)) (fill none) (layer F.Fab)",
        "  )",
    ]

    for pad in pcb.pads:
        pos_x, pos_y = format_coord(pad.position[0]), format_coord(-pad.position[1])
        x_size = format_coord(pad.x_size)
        y_size = format_coord(pad.y_size)
        lines.append(f"  (pad {pad.index} smd oval (at {pos_x} {pos_y}) (size {x_size} {y_size}) (layers F.Cu F.Paste F.Mask))")

    for net in pcb.nets:
        for trace in net.traces:
            layer = get_kicad_layer(trace.layer, total_layers)
            for seg in trace.segments:
                start_x, start_y = format_coord(seg.start[0]), format_coord(-seg.start[1])
                end_x, end_y = format_coord(seg.end[0]), format_coord(-seg.end[1])
                width = seg.width * 1000

                lines.append(f"  (fp_line (start {start_x} {start_y}) (end {end_x} {end_y}) "
                             f"(layer {layer}) (width {width:.3f}))")

    for via in pcb.vias:
        x, y = format_coord(via.position[0]), format_coord(-via.position[1])
        size = format_coord(via.size)
        drill = format_coord(via.drill_size)
        lines.append(f"  (pad \"\" thru_hole circle (at {x} {y}) (size {size} {size}) "
                     f"(drill {drill}) (layers F.Cu B.Cu))")

    # for marker in pcb.markers:
    #     pos_x, pos_y = format_coord(marker.position[0]), format_coord(-marker.position[1])
    #     end_x = format_coord(marker.position[0] + marker.radius)
    #     lines.append(f"  (fp_circle (center {pos_x} {pos_y}) (end {end_x} {pos_y})"
    #                  f"    (stroke (width 0.1) (type default)) (fill none) (layer F.Fab))")

    lines.append(")")

    with open(filename, "w") as f:
        f.write("\n".join(lines))
