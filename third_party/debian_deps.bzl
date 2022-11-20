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
