cc_library(
    name = "dev",
    hdrs = glob(["usr/include/**/*.h"]),
    strip_include_prefix = "usr/include",
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:aarch64",
    ],
    visibility = ["//visibility:public"],
)