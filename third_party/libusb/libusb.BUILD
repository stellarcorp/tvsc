cc_library(
    name = "linux_os_headers",
    hdrs = [
        "libusb/config.h",
        "libusb/libusbi.h",
        "libusb/os/events_posix.h",
        "libusb/os/threads_posix.h",
    ],
    strip_include_prefix = "libusb",
    target_compatible_with = [
        "@platforms//os:linux",
    ],
)

cc_library(
    name = "headers",
    hdrs = glob(["libusb/*.h"]),
    strip_include_prefix = "libusb",
    deps = [
        ":linux_os_headers",
    ],
)

cc_library(
    name = "linux_os",
    srcs = [
        "libusb/os/events_posix.c",
        "libusb/os/linux_netlink.c",
        "libusb/os/linux_udev.c",
        "libusb/os/linux_usbfs.c",
        "libusb/os/threads_posix.c",
    ],
    hdrs = [
        "libusb/os/linux_usbfs.h",
    ],
    target_compatible_with = [
        "@platforms//os:linux",
    ],
    deps = [
        ":headers",
    ] + select({
        "@//platforms:aarch64_linux_build": [
            "@org_debian_ftp_libudev_dev//:dev",
        ],
        # TODO(james): Add other platform selectors as appropriate.
        "//conditions:default": [],
    }),
)

cc_library(
    name = "libusb",
    srcs = glob([
        "libusb/*.c",
    ]),
    hdrs = glob(["libusb/*.h"]),
    visibility = ["//visibility:public"],
    deps = [
        ":headers",
        ":linux_os",
    ],
)
