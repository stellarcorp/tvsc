cc_library(
    name = "headers",
    hdrs = glob(["usr/include/wiringpi2/*.h"]),
    strip_include_prefix = "usr/include/wiringpi2/",
)

cc_library(
    name = "lib",
    visibility = ["//visibility:public"],
    deps = [
        ":headers",
    ],
)
