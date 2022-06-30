filegroup(
    name = "headers",
    srcs = glob(["usr/include/**/*.h"]),
    visibility = ["//visibility:public"],
)

cc_import(
    name = "library",
    static_library = "usr/lib/aarch64-linux-gnu/libavahi-client.a",
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:aarch64",
    ],
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
        ":library",
        "@org_raspbian_archive_multiarch_libavahi_common_dev//:dev",
    ],
)
