load("//third_party/toolchains:toolchains.bzl", "toolchains")
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

    if not native.existing_rule("org_raspbian_archive_multiarch_libzstd"):
        http_archive(
            name = "org_raspbian_archive_multiarch_libzstd",
            urls = [
                "https://archive.raspbian.org/multiarch/pool/main/libz/libzstd/libzstd1_1.4.10%2Bdfsg-1%2Brpi1_arm64.deb",
            ],
            build_file = "//third_party/libzstd:libzstd.BUILD",
            sha256 = "05ba7566384bef212ce12132930d4c3ac4e129de5c67da7573651d916da13003",
            patch_cmds = ["tar xf data.tar.xz"],
        )

    if not native.existing_rule("org_debian_ftp_liblzma"):
        http_archive(
            name = "org_debian_ftp_liblzma",
            urls = [
                "https://ftp.debian.org/debian/pool/main/x/xz-utils/liblzma5_5.2.5-2_arm64.deb",
            ],
            build_file = "//third_party/liblzma:liblzma.BUILD",
            sha256 = "1c7582c7b867b61c0002e9f16bfe035e546d99a9eb7c9a1eae7019ae6d2c1ea7",
            patch_cmds = ["tar xf data.tar.xz"],
        )

    if not native.existing_rule("org_raspbian_archive_multiarch_liblz4"):
        http_archive(
            name = "org_raspbian_archive_multiarch_liblz4",
            urls = [
                "https://archive.raspbian.org/multiarch/pool/main/l/lz4/liblz4-1_1.9.3-1%2Brpi1_arm64.deb",
            ],
            build_file = "//third_party/liblz4:liblz4.BUILD",
            sha256 = "b702839ef3513ca3b5e373863a017449f8765501859c0a2a065ae954986523e9",
            patch_cmds = ["tar xf data.tar.xz"],
        )

    if not native.existing_rule("org_debian_ftp_libssl_dev"):
        http_archive(
            name = "org_debian_ftp_libssl_dev",
            urls = [
                "https://ftp.debian.org/debian/pool/main/o/openssl/libssl-dev_1.1.1n-0+deb11u1_arm64.deb",
            ],
            sha256 = "14877e2e16c0e2a430c44708c90976622c5c42099c4d1b8ccdf64e417296cbb8",
            build_file = "//third_party/openssl:libssl_dev.BUILD",
            patch_cmds = ["tar xf data.tar.xz"],
        )

    if not native.existing_rule("org_debian_ftp_libssl"):
        http_archive(
            name = "org_debian_ftp_libssl",
            urls = [
                "https://ftp.debian.org/debian/pool/main/o/openssl/libssl1.1_1.1.1n-0+deb11u1_arm64.deb",
            ],
            sha256 = "f708c39b20e0bf5f6e6460c196420ffcd91d03048587f90ef4bfad120b773b33",
            build_file = "//third_party/openssl:libssl.BUILD",
            patch_cmds = ["tar xf data.tar.xz"],
        )

    # Hack to work around gRPC's insistence on building boringssl from scratch when we have OpenSSL installed from the
    # OS. Also, we had difficulties getting boringssl and Abseil, one of its dependencies, to compile on arm64.
    if not native.existing_rule("boringssl"):
        native.local_repository(
            name = "boringssl",
            path = "third_party/boringssl",
            repo_mapping = {
                "@openssl": "@org_debian_ftp_libssl_dev",
            },
        )

    if not native.existing_rule("org_debian_ftp_libcap2"):
        http_archive(
            name = "org_debian_ftp_libcap2",
            urls = [
                "https://ftp.debian.org/debian/pool/main/libc/libcap2/libcap2_2.44-1_arm64.deb",
            ],
            build_file = "//third_party/libcap:libcap.BUILD",
            sha256 = "7c5729a1cfd14876685217c5f0545301e7ff1b839262fb487d6a778e8cd8c05a",
            patch_cmds = ["tar xf data.tar.xz"],
        )

    if not native.existing_rule("org_debian_ftp_libgpg-error0"):
        http_archive(
            name = "org_debian_ftp_libgpg-error0",
            urls = [
                "https://ftp.debian.org/debian/pool/main/libg/libgpg-error/libgpg-error0_1.38-2_arm64.deb",
            ],
            build_file = "//third_party/gpg:libgpg-error0.BUILD",
            sha256 = "d1116f4281d6db35279799a21051e0d0e2600d110d7ee2b95b3cca6bec28067c",
            patch_cmds = ["tar xf data.tar.xz"],
        )

    if not native.existing_rule("org_debian_ftp_libgcrypt"):
        http_archive(
            name = "org_debian_ftp_libgcrypt",
            urls = [
                "https://ftp.debian.org/debian/pool/main/libg/libgcrypt20/libgcrypt20_1.8.7-6_arm64.deb",
            ],
            build_file = "//third_party/libgcrypt:libgcrypt.BUILD",
            sha256 = "61ec779149f20923b30adad7bdf4732957e88a5b6a26d94b2210dfe79409959b",
            patch_cmds = ["tar xf data.tar.xz"],
        )

    if not native.existing_rule("org_raspbian_archive_multiarch_libsystemd"):
        http_archive(
            name = "org_raspbian_archive_multiarch_libsystemd",
            urls = [
                "https://archive.raspbian.org/multiarch/pool/main/s/systemd/libsystemd0_247.3-6%2Brpi1_arm64.deb",
            ],
            sha256 = "31ac235de6930c0a769d181f242c27f22c2e34c4de23a31a4595252e57790ed3",
            build_file = "//third_party/systemd:libsystemd.BUILD",
            patch_cmds = ["tar xf data.tar.xz"],
        )

    if not native.existing_rule("org_raspbian_archive_multiarch_libdbus"):
        http_archive(
            name = "org_raspbian_archive_multiarch_libdbus",
            urls = [
                "https://archive.raspbian.org/multiarch/pool/main/d/dbus/libdbus-1-3_1.12.20-1%2Bb3_arm64.deb",
            ],
            sha256 = "8710057dac335cfb08c3383b6baf66c277c6d7e375b33abef5e6a54a41e8d1d6",
            build_file = "//third_party/dbus:libdbus.BUILD",
            patch_cmds = ["tar xf data.tar.xz"],
        )

    if not native.existing_rule("org_raspbian_archive_multiarch_libavahi_client3"):
        http_archive(
            name = "org_raspbian_archive_multiarch_libavahi_client3",
            urls = [
                "https://archive.raspbian.org/multiarch/pool/main/a/avahi/libavahi-client3_0.8-3%2Bb2_arm64.deb",
            ],
            build_file = "//third_party/avahi:libavahi_client3.BUILD",
            sha256 = "107e9dc9060142587f16c4fe4d959472547ed960c79708238aec8c09780448c7",
            patch_cmds = ["tar xf data.tar.xz"],
        )

    if not native.existing_rule("org_raspbian_archive_multiarch_libavahi_common3"):
        http_archive(
            name = "org_raspbian_archive_multiarch_libavahi_common3",
            urls = [
                "https://archive.raspbian.org/multiarch/pool/main/a/avahi/libavahi-common3_0.8-3%2Bb2_arm64.deb",
            ],
            build_file = "//third_party/avahi:libavahi_common3.BUILD",
            sha256 = "2547a18118c623e1ce931e3e98ed7727db537b94107b907830b8bac580809ec6",
            patch_cmds = ["tar xf data.tar.xz"],
        )

    if not native.existing_rule("org_raspbian_archive_multiarch_libavahi_client_dev"):
        http_archive(
            name = "org_raspbian_archive_multiarch_libavahi_client_dev",
            urls = [
                "https://archive.raspbian.org/multiarch/pool/main/a/avahi/libavahi-client-dev_0.8-3%2Bb2_arm64.deb",
            ],
            build_file = "//third_party/avahi:libavahi_client_dev.BUILD",
            sha256 = "79884c061ed37ce69c8a34178d6779cc946a127ffbc6151e2d868ce45c8d225f",
            patch_cmds = ["tar xf data.tar.xz"],
        )

    if not native.existing_rule("org_raspbian_archive_multiarch_libavahi_common_dev"):
        http_archive(
            name = "org_raspbian_archive_multiarch_libavahi_common_dev",
            urls = [
                "https://archive.raspbian.org/multiarch/pool/main/a/avahi/libavahi-common-dev_0.8-3%2Bb2_arm64.deb",
            ],
            build_file = "//third_party/avahi:libavahi_common_dev.BUILD",
            sha256 = "384b127c13d314bf4cf8f587df0a67375ea5b1aba4961838ba1be5f9fe2c949a",
            patch_cmds = ["tar xf data.tar.xz"],
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

    if not native.existing_rule("net_zlib_zlib"):
        http_archive(
            name = "net_zlib_zlib",
            sha256 = "91844808532e5ce316b3c010929493c0244f3d37593afd6de04f71821d5136d9",
            strip_prefix = "zlib-1.2.12",
            urls = [
                "https://zlib.net/zlib-1.2.12.tar.gz",
            ],
            build_file = "//third_party/zlib:zlib.BUILD",
        )

    if not native.existing_rule("com_github_unetworking_usockets"):
        http_archive(
            name = "com_github_unetworking_usockets",
            urls = [
                "https://github.com/uNetworking/uSockets/archive/d8967af421983d40422094e31c54d9f1febeea49.tar.gz",
            ],
            build_file = "//third_party/usockets:usockets.BUILD",
            strip_prefix = "uSockets-d8967af421983d40422094e31c54d9f1febeea49",
        )

    if not native.existing_rule("com_github_unetworking_uwebsockets"):
        http_archive(
            name = "com_github_unetworking_uwebsockets",
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
    _load_bazel_dependencies()
