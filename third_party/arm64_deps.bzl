load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def load_arm64_dependencies():
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
