cc_library(
    name = "soapy_sdr",
    srcs = glob(["lib/**/*.cpp"]) + [
        "lib/TypeHelpers.hpp",
        "lib/ErrorHelpers.hpp",
    ],
    hdrs = glob([
        "include/**/*.h",
        "include/**/*.hpp",
    ]),
    linkopts = ["-ldl"],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)
