load("//third_party/toolchains:toolchains.bzl", "toolchains")
load("//third_party:debian_deps.bzl", "load_debian_dependencies")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def _load_bazel_dependencies():
    if not native.existing_rule("io_bazel_rules_docker"):
        http_archive(
            name = "io_bazel_rules_docker",
            sha256 = "b1e80761a8a8243d03ebca8845e9cc1ba6c82ce7c5179ce2b295cd36f7e394bf",
            urls = ["https://github.com/bazelbuild/rules_docker/releases/download/v0.25.0/rules_docker-v0.25.0.tar.gz"],
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
            sha256 = "5cf189eb6847b4f8fc603a3ffff3b0771c08eec7dd4bd961bfd45477dd13eb73",
            strip_prefix = "googletest-609281088cfefc76f9d0ce82e1ff6c30cc3591e5",
            urls = ["https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip"],
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
            strip_prefix = "glog-0.5.0",
            urls = ["https://github.com/google/glog/archive/v0.5.0.zip"],
        )

    if not native.existing_rule("rules_cc"):
        http_archive(
            name = "rules_cc",
            strip_prefix = "rules_cc-2f8c04c04462ab83c545ab14c0da68c3b4c96191",
            sha256 = "4aeb102efbcfad509857d7cb9c5456731e8ce566bfbf2960286a2ec236796cc3",
            urls = [
                "https://github.com/bazelbuild/rules_cc/archive/2f8c04c04462ab83c545ab14c0da68c3b4c96191.tar.gz",
            ],
        )

    if not native.existing_rule("rules_proto"):
        http_archive(
            name = "rules_proto",
            sha256 = "e017528fd1c91c5a33f15493e3a398181a9e821a804eb7ff5acdd1d2d6c2b18d",
            strip_prefix = "rules_proto-4.0.0-3.20.0",
            urls = [
                "https://github.com/bazelbuild/rules_proto/archive/refs/tags/4.0.0-3.20.0.tar.gz",
            ],
        )

    if not native.existing_rule("bazel_skylib"):
        http_archive(
            name = "bazel_skylib",
            urls = [
                "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.2.1/bazel-skylib-1.2.1.tar.gz",
                "https://github.com/bazelbuild/bazel-skylib/releases/download/1.2.1/bazel-skylib-1.2.1.tar.gz",
            ],
            sha256 = "f7be3474d42aae265405a592bb7da8e171919d74c16f082a5457840f06054728",
        )

    # Hack to work around gRPC's insistence on building boringssl from scratch when we have OpenSSL installed from the
    # OS. Also, we had difficulties getting boringssl and Abseil, one of its dependencies, to compile on arm64.
    if not native.existing_rule("boringssl"):
        native.local_repository(
            name = "boringssl",
            path = "third_party/boringssl",
        )

    if not native.existing_rule("com_github_grpc_grpc"):
        http_archive(
            name = "com_github_grpc_grpc",
            urls = [
                "https://github.com/grpc/grpc/archive/refs/tags/v1.47.0.tar.gz",
            ],
            sha256 = "271bdc890bf329a8de5b65819f0f9590a5381402429bca37625b63546ed19e54",
            strip_prefix = "grpc-1.47.0",
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

    if not native.existing_rule("com_github_unetworking_usockets"):
        http_archive(
            name = "com_github_unetworking_usockets",
            sha256 = "7cd756edaa70efa58137cae3ab96d8ea15febbf2c92bccbc65b2b9afc322bcff",
            urls = [
                "https://github.com/uNetworking/uSockets/archive/d8967af421983d40422094e31c54d9f1febeea49.tar.gz",
            ],
            build_file = "//third_party/usockets:usockets.BUILD",
            strip_prefix = "uSockets-d8967af421983d40422094e31c54d9f1febeea49",
        )

    if not native.existing_rule("com_github_unetworking_uwebsockets"):
        http_archive(
            name = "com_github_unetworking_uwebsockets",
            sha256 = "6b8ea09ab58c4c9aff0e9b7a8ada5ac142bc96ae5cc4e6a89aeaacdb94ef2fad",
            urls = [
                "https://github.com/uNetworking/uWebSockets/archive/refs/tags/v20.23.0.tar.gz",
            ],
            build_file = "//third_party/uwebsockets:uwebsockets.BUILD",
            strip_prefix = "uWebSockets-20.23.0",
        )

    if not native.existing_rule("com_github_osmocom_rtl_sdr"):
        http_archive(
            name = "com_github_osmocom_rtl_sdr",
            sha256 = "ee10a76fe0c6601102367d4cdf5c26271e9442d0491aa8df27e5a9bf639cff7c",
            urls = [
                "https://github.com/osmocom/rtl-sdr/archive/refs/tags/0.6.0.tar.gz",
            ],
            build_file = "//third_party/rtl_sdr:rtl_sdr.BUILD",
            strip_prefix = "rtl-sdr-0.6.0",
            patches = ["//third_party/rtl_sdr:include_path.patch"],
        )

    if not native.existing_rule("com_github_pothosware_soapyrtlsdr"):
        http_archive(
            name = "com_github_pothosware_soapyrtlsdr",
            sha256 = "757c3c3bd17c5a12c7168db2f2f0fd274457e65f35e23c5ec9aec34e3ef54ece",
            urls = [
                "https://github.com/pothosware/SoapyRTLSDR/archive/refs/tags/soapy-rtl-sdr-0.3.3.tar.gz",
            ],
            build_file = "//third_party/soapy_rtlsdr:soapy_rtlsdr.BUILD",
            strip_prefix = "SoapyRTLSDR-soapy-rtl-sdr-0.3.3",
            patches = ["//third_party/soapy_rtlsdr:module_version.cpp.patch"],
        )

    if not native.existing_rule("com_gitlab_tvsc_soapysdr"):
        http_archive(
            name = "com_gitlab_tvsc_soapysdr",
            urls = ["https://gitlab.com/tvsc/SoapySDR/-/archive/tvsc/SoapySDR-tvsc.zip"],
            strip_prefix = "SoapySDR-tvsc",
            sha256 = "c16ed2dd19bd6efd1dc62591eaa334696c7c3e92ea1c8f794592746d0f94f50c",
        )

def load_dependencies():
    toolchains()
    load_debian_dependencies()
    _load_bazel_dependencies()
