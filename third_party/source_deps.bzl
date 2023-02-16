load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def load_source_dependencies():
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
                "https://github.com/grpc/grpc/archive/refs/tags/v1.47.0.tar.gz",
            ],
            sha256 = "271bdc890bf329a8de5b65819f0f9590a5381402429bca37625b63546ed19e54",
            strip_prefix = "grpc-1.47.0",
            patches = ["//third_party/grpc:grpc_build.patch"],
        )

    if not native.existing_rule("com_github_unetworking_usockets"):
        http_archive(
            name = "com_github_unetworking_usockets",
            sha256 = "302f523448c9130bdd9802ca607578478172eb7f1713796c98284d4369924c23",
            urls = [
                "https://github.com/uNetworking/uSockets/archive/b950efd6b10f06dd3ecb5b692e5d415f48474647.tar.gz",
            ],
            build_file = "//third_party/usockets:usockets.BUILD",
            strip_prefix = "uSockets-b950efd6b10f06dd3ecb5b692e5d415f48474647",
        )

    if not native.existing_rule("com_github_unetworking_uwebsockets"):
        http_archive(
            name = "com_github_unetworking_uwebsockets",
            sha256 = "cda266f7ed6abe67ef3cae6e223a580fe5091db9156c1f4123ee328ae21511c9",
            urls = [
                "https://github.com/uNetworking/uWebSockets/archive/refs/tags/v20.36.0.tar.gz",
            ],
            build_file = "//third_party/uwebsockets:uwebsockets.BUILD",
            strip_prefix = "uWebSockets-20.36.0",
            patches = ["//third_party/uwebsockets:Loop.h.patch"],
        )

    if not native.existing_rule("com_github_bxparks_epoxy_duino"):
        http_archive(
            name = "com_github_bxparks_epoxy_duino",
            sha256 = "c05755aacd52c02b42b9908554d2cf185d8a5d37ed8ac3f393ed833566608b40",
            urls = [
                "https://github.com/bxparks/EpoxyDuino/archive/refs/tags/v1.5.0.tar.gz",
            ],
            strip_prefix = "EpoxyDuino-1.5.0",
            build_file = "//third_party/epoxy_duino:epoxy_duino.BUILD",
            patches = [
                "//third_party/epoxy_duino:arduino.h.patch",
                "//third_party/epoxy_duino:arduino.cpp.patch",
            ],
        )

    if not native.existing_rule("com_airspayce_mikem_radio_head"):
        http_archive(
            name = "com_airspayce_mikem_radio_head",
            sha256 = "73400cce6aa7bfb6bfc7acd640f7c9b2354f7d755a92b38c2be814bff85cc44b",
            urls = [
                "https://www.airspayce.com/mikem/arduino/RadioHead/RadioHead-1.121.zip",
            ],
            build_file = "//third_party/radio_head:radio_head.BUILD",
            patches = ["//third_party/radio_head:ATOMIC_BLOCK.patch"],
        )
