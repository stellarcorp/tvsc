licenses(["notice"])

cc_library(
    name = "rtl_sdr_hdrs",
    hdrs = glob([
        "include/**/*.h",
    ]),
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "rtl_sdr",
    srcs = glob(
        [
            "src/**/*.c",
            "src/**/*.h",
        ],
        exclude = [
            "src/rtl_adsb.c",
            "src/rtl_biast.c",
            "src/rtl_eeprom.c",
            "src/rtl_fm.c",
            "src/rtl_power.c",
            "src/rtl_sdr.c",
            "src/rtl_tcp.c",
            "src/rtl_test.c",
        ],
    ),
    copts = ["-I/usr/include/libusb-1.0"],
    visibility = ["//visibility:public"],
    deps = [":rtl_sdr_hdrs"],
)
