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

filegroup(
    name = "teensy40_linker_script",
    srcs = ["avr/cores/teensy4/imxrt1062.ld"],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "teensy41_linker_script",
    srcs = ["avr/cores/teensy4/imxrt1062_t41.ld"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "spi_headers",
    hdrs = [
        "avr/libraries/SPI/SPI.h",
    ],
    strip_include_prefix = "avr/libraries/SPI",
    target_compatible_with = [
        "@platforms//os:none",
        "@platforms//cpu:armv7e-mf",
    ],
)

cc_library(
    name = "spi",
    srcs = [
        "avr/libraries/SPI/SPI.cpp",
    ],
    target_compatible_with = [
        "@platforms//os:none",
        "@platforms//cpu:armv7e-mf",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":spi_headers",
        ":teensy4_core",
    ],
)
