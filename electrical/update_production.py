#!/usr/bin/env python

from pathlib import Path

all_directories = ["1s1p_left_connectors", "1s1p_right_connectors", "2s1p_battery_board", "bridge_core_board", "power_board", "solar_debug_board"]

def move_directory_contents_up(dir):
    if not dir.is_dir():
        raise RuntimeError("Can't move directory contents. No such directory '{}'.".format(dir))
    dest_dir = dir.parent
    for src in dir.iterdir():
        dest = dest_dir.joinpath(src.name)
        src.rename(dest)


def remove_directory(dir):
    for file in dir.iterdir():
        if file.is_dir():
            remove_directory(file)
        else:
            file.unlink()
    dir.rmdir()


def find_all_production_directories(parent_dir):
    if not parent_dir.is_dir():
        raise RuntimeError("Can't read production directories. No such directory '{}'.".format(parent_dir))
    dir_name = parent_dir.joinpath("production")
    return [x for x in dir_name.iterdir() if x.is_dir()]


def sort_production_directories_latest_first(directory_list):
    directory_list.sort(reverse=True)


if __name__ == "__main__":
    cwd = Path(".")
    base_dir = cwd.joinpath("pcb_design")
    for dir in all_directories:
        production_dirs = find_all_production_directories(base_dir.joinpath(dir))
        if production_dirs:
            sort_production_directories_latest_first(production_dirs)
            move_directory_contents_up(production_dirs[0])
            for production_dir in production_dirs:
                remove_directory(production_dir)
            print("Updated production artifacts under '{}'.".format(dir))
        else:
            print("No production artifacts found in directory '{}'. Ignoring.".format(dir))
