load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def load_source_dependencies():
    if not native.existing_rule("io_bazel_rules_closure"):
        http_archive(
            name = "io_bazel_rules_closure",
            sha256 = "12f1169ebe782f8631fcb8da81aa12ba192f8952875b4a1e8f20c49fc6e10223",
            strip_prefix = "rules_closure-0.14.0",
            urls = [
                "https://mirror.bazel.build/github.com/bazelbuild/rules_closure/archive/0.14.0.tar.gz",
                "https://github.com/bazelbuild/rules_closure/archive/0.14.0.tar.gz",
            ],
        )

    if not native.existing_rule("rules_proto"):
        http_archive(
            name = "rules_proto",
            sha256 = "6fb6767d1bef535310547e03247f7518b03487740c11b6c6adb7952033fe1295",
            strip_prefix = "rules_proto-6.0.2",
            url = "https://github.com/bazelbuild/rules_proto/releases/download/6.0.2/rules_proto-6.0.2.tar.gz",
        )

    if not native.existing_rule("rules_pkg"):
        http_archive(
            name = "rules_pkg",
            urls = [
                "https://mirror.bazel.build/github.com/bazelbuild/rules_pkg/releases/download/0.7.0/rules_pkg-0.7.0.tar.gz",
                "https://github.com/bazelbuild/rules_pkg/releases/download/0.7.0/rules_pkg-0.7.0.tar.gz",
            ],
            sha256 = "8a298e832762eda1830597d64fe7db58178aa84cd5926d76d5b744d6558941c2",
        )

    if not native.existing_rule("com_google_googletest"):
        http_archive(
            name = "com_google_googletest",
            sha256 = "3c2a0999fb9f85423d17e3000b339fd28501433f70c2e9941c8e784ac8624a10",
            strip_prefix = "googletest-cead3d57c93ff8c4e5c1bbae57a5c0b0b0f6e168",
            urls = ["https://github.com/google/googletest/archive/cead3d57c93ff8c4e5c1bbae57a5c0b0b0f6e168.zip"],
        )

    if not native.existing_rule("com_github_google_benchmark"):
        http_archive(
            name = "com_github_google_benchmark",
            sha256 = "2a778d821997df7d8646c9c59b8edb9a573a6e04c534c01892a40aa524a7b68c",
            strip_prefix = "benchmark-bf585a2789e30585b4e3ce6baf11ef2750b54677",
            urls = ["https://github.com/google/benchmark/archive/bf585a2789e30585b4e3ce6baf11ef2750b54677.zip"],
        )

    if not native.existing_rule("com_github_gflags_gflags"):
        http_archive(
            name = "com_github_gflags_gflags",
            sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
            strip_prefix = "gflags-2.2.2",
            urls = ["https://github.com/gflags/gflags/archive/v2.2.2.tar.gz"],
        )

    if not native.existing_rule("com_github_google_glog"):
        http_archive(
            name = "com_github_google_glog",
            sha256 = "21bc744fb7f2fa701ee8db339ded7dce4f975d0d55837a97be7d46e8382dea5a",
            urls = ["https://github.com/google/glog/archive/v0.5.0.zip"],
            strip_prefix = "glog-0.5.0",
        )

    if not native.existing_rule("zlib"):
        http_archive(
            name = "zlib",
            sha256 = "1525952a0a567581792613a9723333d7f8cc20b87a81f920fb8bc7e3f2251428",
            strip_prefix = "zlib-1.2.13",
            urls = [
                "https://github.com/madler/zlib/archive/refs/tags/v1.2.13.tar.gz",
            ],
            build_file = "//third_party/zlib:zlib.BUILD",
        )

    if not native.existing_rule("com_github_grpc_grpc"):
        http_archive(
            name = "com_github_grpc_grpc",
            urls = [
                "https://github.com/grpc/grpc/archive/refs/tags/v1.52.1.tar.gz",
            ],
            sha256 = "ec125d7fdb77ecc25b01050a0d5d32616594834d3fe163b016768e2ae42a2df6",
            strip_prefix = "grpc-1.52.1",
        )

    # This package includes all of the source code for supporting C/C++ applications on Teensy
    # platforms, including Arduino.h.
    # See note in toolchains.bzl for version information of this package. This package should be
    # kept in sync with the Teensy toolchains in toolchains.bzl.
    # if not native.existing_rule("com_pjrc_teensy_package"):
    #     http_archive(
    #         name = "com_pjrc_teensy_package",
    #         sha256 = "b9c1ea852bd5ac625559685a864d40803df566650864b415e7c14c9623520c59",
    #         urls = [
    #             "https://www.pjrc.com/teensy/td_159/teensy-package-1.59.0.tar.zst",
    #         ],
    #         build_file = "//third_party/arduino/teensyduino:teensy_package.BUILD",
    #         patches = ["//third_party/arduino/teensyduino:Time.cpp.patch"],
    #     )

    # This package includes the source code for STM32 drivers for various facilities.
    if not native.existing_rule("com_github_stellarcorp_stm32_libraries"):
        http_archive(
            name = "com_github_stellarcorp_stm32_libraries",
            sha256 = "28f7fa863f02ae98a712313fbf05b03548f6d62a54cb472f32e9c60ac5af65d0",
            urls = [
                "https://github.com/stellarcorp/stm32_libraries/archive/4671e6cabe101c109cf5f745e6a52166fa87641a.tar.gz",
            ],
            strip_prefix = "stm32_libraries-4671e6cabe101c109cf5f745e6a52166fa87641a",
        )
