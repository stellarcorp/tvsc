cc_library(
    name = "original_headers",
    hdrs = glob(["usr/include/wiringpi2/*.h"]),
    strip_include_prefix = "usr/include/wiringpi2/",
)

cc_library(
    name = "arduino_header",
    hdrs = ["usr/include/Arduino.h"],
    strip_include_prefix = "usr/include/",
)

cc_library(
    name = "lib",
    visibility = ["//visibility:public"],
    deps = [
        ":arduino_header",
        ":original_headers",
    ],
)
