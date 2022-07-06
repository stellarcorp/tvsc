cc_library(
    name = "conf",
    hdrs = glob(["usr/include/aarch64-linux-gnu/**/*.h"]),
    strip_include_prefix = "usr/include/aarch64-linux-gnu",
)

cc_library(
    name = "dev",
    hdrs = glob(["usr/include/**/*.h"]),
    strip_include_prefix = "usr/include",
    visibility = ["//visibility:public"],
    deps = [
        ":conf",
        "@org_debian_ftp_libssl//:libcrypto",
        "@org_debian_ftp_libssl//:libssl",
    ],
)

cc_library(
    name = "ssl",
    visibility = ["//visibility:public"],
    deps = [
        ":dev",
    ],
)

cc_library(
    name = "crypto",
    visibility = ["//visibility:public"],
    deps = [
        ":dev",
    ],
)
