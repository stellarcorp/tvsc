cc_library(
    name = "uwebsockets",
    hdrs = glob(["src/**/*.h"]),
    strip_include_prefix = "/src",
    visibility = ["//visibility:public"],
    deps = [
        "@com_github_unetworking_usockets//:usockets",
        "@net_zlib_zlib//:zlib",
    ],
)
