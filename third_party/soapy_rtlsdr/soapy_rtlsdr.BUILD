licenses(["notice"])

cc_library(
    name = "soapy_rtlsdr",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    linkstatic = True,
    visibility = ["//visibility:public"],
    deps = [
        "@com_github_osmocom_rtl_sdr//:rtl_sdr",
        "@com_gitlab_tvsc_soapysdr//:soapy",
    ],
)
