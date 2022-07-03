filegroup(
    name = "headers",
    srcs = glob(["usr/include/**/*.h"]),
    visibility = ["//visibility:public"],
)

cc_library(
    name = "dev",
    hdrs = glob(["usr/include/**/*.h"]),
    strip_include_prefix = "usr/include",
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:aarch64",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@org_raspbian_archive_multiarch_libavahi_common_dev//:dev",
    ],
    linkopts = ["-lavahi-client"],
)
