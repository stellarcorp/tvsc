cc_library(
    name = "usockets",
    srcs = glob(["src/**/*.c"]),
    hdrs = glob(["src/**/*.h"]),
    strip_include_prefix = "/src",
    visibility = ["//visibility:public"],
    local_defines = ["LIBUS_NO_SSL"],
)
