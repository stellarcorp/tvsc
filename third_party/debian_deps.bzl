load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("//third_party:tvsc_archive.bzl", "tvsc_archive")

def load_debian_dependencies():
    # zstd
    tvsc_archive(
        name = "org_debian_ftp_libzstd_dev",
        libname = "libzstd-dev",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/libz/libzstd/libzstd-dev_1.4.4+dfsg-3~bpo10+1_arm64.deb",
        ],
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/libz/libzstd/libzstd-dev_1.4.4+dfsg-3~bpo10+1_amd64.deb",
        ],
        x86_64_sha256 = "5071db2db093534270f1d83ace0bf25066ba4a376e2d52446dda5ec7a7dee83d",
    )

    tvsc_archive(
        name = "org_debian_ftp_libzstd1",
        libname = "libzstd1",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/libz/libzstd/libzstd1_1.4.4+dfsg-3~bpo10+1_arm64.deb",
        ],
        arm64_library_paths = {"libzstd": "usr/lib/aarch64-linux-gnu/libzstd.so.1.4.4"},
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/libz/libzstd/libzstd1_1.4.4+dfsg-3~bpo10+1_amd64.deb",
        ],
        x86_64_library_paths = {"libzstd": "usr/lib/x86_64-linux-gnu/libzstd.so.1.4.4"},
        x86_64_sha256 = "94f2b4f413cb83aed3fbb2acce385d94c3ff04aa30db8416c12387e5a2fdfd73",
    )

    # OpenSSL
    tvsc_archive(
        name = "org_debian_ftp_libssl_dev",
        libname = "libssl-dev",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/o/openssl/libssl-dev_1.1.1n-0+deb11u3_arm64.deb",
        ],
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/o/openssl/libssl-dev_1.1.1n-0+deb11u3_amd64.deb",
        ],
        x86_64_sha256 = "31ea36803b8d249a9c8a3769f77ac51238e122089415432809de46c84350b5c1",
        deps = ["@org_debian_ftp_libssl//:lib"],
    )

    tvsc_archive(
        name = "org_debian_ftp_libssl",
        libname = "libssl1.1",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/o/openssl/libssl1.1_1.1.1n-0+deb11u1_arm64.deb",
        ],
        arm64_library_paths = {
            "libssl": "usr/lib/aarch64-linux-gnu/libssl.so.1.1",
            "libcrypto": "usr/lib/aarch64-linux-gnu/libcrypto.so.1.1",
            "afalg_engine": "usr/lib/aarch64-linux-gnu/engines-1.1/afalg.so",
            "padlock_engine": "usr/lib/aarch64-linux-gnu/engines-1.1/padlock.so",
        },
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/o/openssl/libssl1.1_1.1.1n-0+deb11u3_amd64.deb",
        ],
        x86_64_library_paths = {
            "libssl": "usr/lib/x86_64-linux-gnu/libssl.so.1.1",
            "libcrypto": "usr/lib/x86_64-linux-gnu/libcrypto.so.1.1",
            "afalg_engine": "usr/lib/x86_64-linux-gnu/engines-1.1/afalg.so",
            "padlock_engine": "usr/lib/x86_64-linux-gnu/engines-1.1/padlock.so",
        },
        x86_64_sha256 = "c3480bf90725c993187de02e5144e90ea81226e7e2538ba24687f5a0adc8ca5c",
    )

    # Avahi
    tvsc_archive(
        name = "org_debian_ftp_libavahi_client_dev",
        libname = "libavahi-client-dev",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-client-dev_0.8-5+deb11u1_arm64.deb",
        ],
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-client-dev_0.8-5+deb11u1_amd64.deb",
        ],
        x86_64_sha256 = "2d08090e7f17a1b2ae3e6d5cc79f46707ecbabf3a3952b6b128829249b3ad475",
        deps = [
            "@org_debian_ftp_libavahi_client3//:lib",
            "@org_debian_ftp_libavahi_common3//:lib",
            "@org_debian_ftp_libavahi_common_dev//:lib",
        ],
    )

    tvsc_archive(
        name = "org_debian_ftp_libavahi_client3",
        libname = "libavahi-client3",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-client3_0.8-5+deb11u1_arm64.deb",
        ],
        arm64_library_paths = {
            "libavahi-client3": "usr/lib/aarch64-linux-gnu/libavahi-client.so.3",
        },
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-client3_0.8-5+deb11u1_amd64.deb",
        ],
        x86_64_sha256 = "44104ae278d853f9d20b90a6192257497d430f3ff4af093af1c504effb9caf4f",
        x86_64_library_paths = {
            "libavahi-client3": "usr/lib/x86_64-linux-gnu/libavahi-client.so.3",
        },
        deps = [
            "@org_debian_ftp_libavahi_common3//:lib",
        ],
    )

    tvsc_archive(
        name = "org_debian_ftp_libavahi_common_dev",
        libname = "libavahi-common-dev",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-common-dev_0.8-5+deb11u1_arm64.deb",
        ],
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-common-dev_0.8-5+deb11u1_amd64.deb",
        ],
        x86_64_sha256 = "1bb6db35a25b49e69c3208e2e05e10b2ee47383154de7f99e595032cf6a378f5",
        deps = [
            "@org_debian_ftp_libavahi_common3//:lib",
        ],
    )

    tvsc_archive(
        name = "org_debian_ftp_libavahi_common3",
        libname = "libavahi-common3",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-common3_0.8-5+deb11u1_arm64.deb",
        ],
        arm64_library_paths = {
            "libavahi-common3": "usr/lib/aarch64-linux-gnu/libavahi-common.so.3",
        },
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-common3_0.8-5+deb11u1_amd64.deb",
        ],
        x86_64_sha256 = "d5d97f84a894e6ef0e535a17d1dcc1ed64933d6e04a350306e989d05b37de00c",
        x86_64_library_paths = {
            "libavahi-common3": "usr/lib/x86_64-linux-gnu/libavahi-common.so.3",
        },
    )

    # libusb
    tvsc_archive(
        name = "org_debian_ftp_libusb_dev",
        libname = "libusb-dev",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/libu/libusb-1.0/libusb-1.0-0-dev_1.0.26-1_arm64.deb",
        ],
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/libu/libusb-1.0/libusb-1.0-0-dev_1.0.26-1_amd64.deb",
        ],
        x86_64_sha256 = "22a952f48cdb59f1d0c476874facc62418eda5b4c30c5a23a17c28781b426c1b",
        deps = [
            "@org_debian_ftp_libusb//:lib",
        ],
    )

    tvsc_archive(
        name = "org_debian_ftp_libusb",
        libname = "libusb",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/libu/libusb-1.0/libusb-1.0-0_1.0.26-1_arm64.deb",
        ],
        arm64_library_paths = {
            "libusb": "usr/lib/aarch64-linux-gnu/libusb-1.0.so.0",
        },
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/libu/libusb-1.0/libusb-1.0-0_1.0.26-1_amd64.deb",
        ],
        x86_64_library_paths = {
            "libusb": "usr/lib/x86_64-linux-gnu/libusb-1.0.so.0",
        },
        x86_64_sha256 = "0a8a6c4a7d944538f2820cbde2a313f2fe6f94c21ffece9e6f372fc2ab8072e1",
    )
