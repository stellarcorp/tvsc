cc_import(
    name = "libdbus",
    shared_library = "lib/aarch64-linux-gnu/libdbus-1.so.3",
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:arm64",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@org_raspbian_archive_multiarch_libsystemd//:libsystemd",
    ],
)
