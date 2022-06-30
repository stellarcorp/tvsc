cc_import(
    name = "libgcrypt",
    shared_library = "usr/lib/aarch64-linux-gnu/libgcrypt.so.20",
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:aarch64",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@org_debian_ftp_libgpg-error0//:libgpg-error0",
    ],
)
