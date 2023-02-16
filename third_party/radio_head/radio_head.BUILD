cc_library(
    name = "driver_base",
    srcs = [
        "RadioHead/RHGenericDriver.cpp",
    ],
    hdrs = [
        "RadioHead/RHGenericDriver.h",
        "RadioHead/RHutil/atomic.h",
        "RadioHead/RadioHead.h",
    ],
    defines = [
        "RH_PLATFORM_ARDUINO",
        "ARDUINO=155",
    ],
    strip_include_prefix = "RadioHead/",
    visibility = ["//visibility:public"],
    deps = [
        "@com_github_bxparks_epoxy_duino//:core",
    ],
)

cc_library(
    name = "rf69_driver",
    srcs = [
        "RadioHead/RHGenericSPI.cpp",
        "RadioHead/RHHardwareSPI.cpp",
        "RadioHead/RHSPIDriver.cpp",
        "RadioHead/RH_RF69.cpp",
    ],
    hdrs = [
        "RadioHead/RHGenericSPI.h",
        "RadioHead/RHHardwareSPI.h",
        "RadioHead/RHSPIDriver.h",
        "RadioHead/RH_RF69.h",
    ],
    strip_include_prefix = "RadioHead/",
    visibility = ["//visibility:public"],
    deps = [
        ":driver_base",
        "@com_github_bxparks_epoxy_duino//:core",
    ],
)
