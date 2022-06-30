cc_library(
    name = "rtl_sdr",
    srcs = glob([
        "src/**/*.c",
        "src/**/*.h",
    ]),
    hdrs = glob(["include/**/*.h"]),
    strip_include_prefix = "include/",
    visibility = ["//visibility:public"],
    deps = [
        "@com_github_libusb_libusb//:libusb",
    ],
)
