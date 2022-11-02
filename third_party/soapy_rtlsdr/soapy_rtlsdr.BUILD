licenses(["notice"])

cc_library(
    name = "soapy_rtlsdr_lib",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "@com_github_osmocom_rtl_sdr//:rtl_sdr",
        "@com_gitlab_tvsc_soapysdr//:soapy",
    ],
)

cc_library(
    name = "soapy_rtlsdr",
    visibility = ["//visibility:public"],
    deps = [
        ":soapy_rtlsdr_lib",
    ],
)
