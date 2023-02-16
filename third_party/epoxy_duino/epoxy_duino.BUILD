cc_library(
    name = "core",
    srcs = glob(
        ["cores/**/*.cpp"],
        exclude = ["cores/epoxy/Esp.cpp"],
    ),
    hdrs = glob(
        ["cores/**/*.h"],
        exclude = ["cores/epoxy/Esp.h"],
    ),
    strip_include_prefix = "/cores/epoxy",
    visibility = ["//visibility:public"],
)
