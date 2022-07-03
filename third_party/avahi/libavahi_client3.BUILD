cc_import(
    name = "lib",
    shared_library = "usr/lib/aarch64-linux-gnu/libavahi-client.so.3",
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:arm64",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@org_raspbian_archive_multiarch_libdbus//:libdbus",
    ],
)
