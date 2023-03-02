cc_library(
    name = "teensy4_core_headers",
    hdrs = glob(["avr/cores/teensy4/**/*.h"]),
    strip_include_prefix = "avr/cores/teensy4",
    target_compatible_with = [
        "@platforms//os:none",
        "@platforms//cpu:armv7e-mf",
    ],
)

cc_library(
    name = "teensy4_core",
    srcs = glob([
        "avr/cores/teensy4/**/*.c",
        "avr/cores/teensy4/**/*.cpp",
    ]),
    target_compatible_with = [
        "@platforms//os:none",
        "@platforms//cpu:armv7e-mf",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":teensy4_core_headers",
    ],
)
