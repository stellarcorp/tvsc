filegroup(
    name = "socket_defs_configuration",
    srcs = select(
        {
            # Note: add other platforms as needed to this list. To add files for other platforms, use the patches
            # and similar sections of the http_archive() rule that brings in this package.
            "@platforms//os:linux": [
                "common/SoapySocketDefs.linux.hpp",
            ],
            "@platforms//os:macos": [
            ],
            "@platforms//os:windows": [
            ],
            "//conditions:default": [],
        },
    ),
)

genrule(
    # Copy the platform-specific SoapySocketDefs.<platform>.hpp file to SoapySocketDefs.hpp.
    name = "SoapySocketDefs",
    srcs = [":socket_defs_configuration"],
    outs = ["common/SoapySocketDefs.hpp"],
    cmd_bash = "cp $< $@",
)

cc_library(
    name = "common",
    srcs = glob(
        ["common/**/*.cpp"],
        exclude = [
            "common/SoapyMDNSEndpointApple.cpp",
            # Note: to enable discovery via Avahi, keep SoapyMDNSEndpointAvahi.cpp in the build, and exclude
            # SoapyMDNSEndpointNone.cpp instead. Also, the //third_party/soapy:soapy_remote_common target may need to
            # depend on Avahi.
            "common/SoapyMDNSEndpointAvahi.cpp",
            "common/SoapyIfAddrsWindows.cpp",
        ],
    ),
    hdrs = glob(["common/**/*.hpp"]) + [":SoapySocketDefs"],
    strip_include_prefix = "common",
    visibility = ["//visibility:public"],
    deps = [
        "@com_github_pothosware_soapysdr//:soapy_sdr",
    ],
)

cc_library(
    name = "server",
    srcs = glob(
        ["server/**/*.cpp"],
        exclude = ["server/ThreadPrioWindows.cpp"],
    ),
    hdrs = glob(["server/**/*.hpp"]),
    strip_include_prefix = "server",
    visibility = ["//visibility:public"],
    deps = [
        ":common",
        "@com_github_pothosware_soapysdr//:soapy_sdr",
    ],
)
