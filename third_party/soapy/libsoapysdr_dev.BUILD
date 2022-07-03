cc_library(
    name = "dev",
    hdrs = glob(["usr/include/**/*.h"]) + glob(["usr/include/**/*.hpp"]),
    strip_include_prefix = "usr/include",
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:aarch64",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@org_debian_ftp_libsoapysdr//:lib",
    ],
)
