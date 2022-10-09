cc_library(
    name = "uwebsockets",
    hdrs = glob(["src/**/*.h"]),
    # TODO(james): [https://gitlab.com/tvsc/tvsc/-/issues/8] Integrate zlib or libdeflate.
    defines = ["UWS_NO_ZLIB", "UWS_MOCK_ZLIB"],
    strip_include_prefix = "/src",
    visibility = ["//visibility:public"],
    deps = [
        "@com_github_unetworking_usockets//:usockets",
    ],
)
