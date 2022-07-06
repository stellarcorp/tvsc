cc_library(
    name = "dev",
    hdrs = glob(["usr/include/**/*.h"]) + glob(["usr/include/**/*.hpp"]),
    strip_include_prefix = "usr/include",
    # TODO(james): Re-enable platform support when gRPC and Abseil enable them.
    # target_compatible_with = [
    #     "@platforms//os:linux",
    #     "@platforms//cpu:arm64",
    # ],
    visibility = ["//visibility:public"],
    deps = [
        "@org_debian_ftp_libsoapysdr//:lib",
    ],
)
