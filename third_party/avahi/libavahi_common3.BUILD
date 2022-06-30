cc_import(
    name = "libraries",
    shared_library = "usr/lib/aarch64-linux-gnu/libavahi-common.so.3",
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:aarch64",
    ],
)

cc_library(
    name = "libavahi_common3",
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:aarch64",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":libraries",
        "@org_raspbian_archive_multiarch_libavahi_common_dev//:dev",
    ],
)
