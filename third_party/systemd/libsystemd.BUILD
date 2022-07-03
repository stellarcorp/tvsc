cc_import(
    name = "libsystemd",
    shared_library = "usr/lib/aarch64-linux-gnu/libsystemd.so.0",
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:arm64",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@org_debian_ftp_libcap2//:libcap",
        "@org_debian_ftp_liblzma//:liblzma",
        "@org_debian_ftp_libgcrypt//:libgcrypt",
        "@org_raspbian_archive_multiarch_liblz4//:liblz4",
        "@org_raspbian_archive_multiarch_libzstd//:libzstd",
    ],
)
