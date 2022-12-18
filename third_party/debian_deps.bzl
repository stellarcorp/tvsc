load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("//third_party:tvsc_archive.bzl", "tvsc_archive")

# Load Debian packages as prebuilt dependencies. The exact packages should come from
# https://ftp.debian.org/debian/pool/main, and ideally, we use the exact same version for every
# platform.
def load_debian_dependencies():
    # zstd
    tvsc_archive(
        name = "org_debian_ftp_libzstd_dev",
        libname = "libzstd-dev",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/libz/libzstd/libzstd-dev_1.4.4+dfsg-3~bpo10+1_arm64.deb",
        ],
        arm64_sha256 = "fea90941e0ea8875737a55a5b3a538ca33b41343bb5309f17f8012f6f61f2cb4",
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
        arm64_sha256 = "b1b2f11a92c2dbd60d6c63eb690dd877e9dbcfb4d3c8c33caa9595d2f7bb5945",
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
        arm64_sha256 = "684bbc6061f44d58ed3133377cca902763c79a095f75d5e2458a45b9db3b1e1d",
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
            "https://ftp.debian.org/debian/pool/main/o/openssl/libssl1.1_1.1.1n-0+deb11u3_arm64.deb",
        ],
        arm64_library_paths = {
            "libssl": "usr/lib/aarch64-linux-gnu/libssl.so.1.1",
            "libcrypto": "usr/lib/aarch64-linux-gnu/libcrypto.so.1.1",
        },
        arm64_sha256 = "8d4ef5a06719fabd90a6a27729ade7b9dde1a1c923a89ba1406af6489fa7c82f",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/o/openssl/libssl1.1_1.1.1n-0+deb11u3_amd64.deb",
        ],
        x86_64_library_paths = {
            "libssl": "usr/lib/x86_64-linux-gnu/libssl.so.1.1",
            "libcrypto": "usr/lib/x86_64-linux-gnu/libcrypto.so.1.1",
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
        arm64_sha256 = "be0d9c2b5ac02deac4c356f309071f3755361b53e743dd9f837e526c27b21504",
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
        arm64_sha256 = "cba67afc387284176d4fdc1d44b07f51e8a182786ed3babda65f957110ce9745",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-client3_0.8-5+deb11u1_amd64.deb",
        ],
        x86_64_sha256 = "44104ae278d853f9d20b90a6192257497d430f3ff4af093af1c504effb9caf4f",
        x86_64_library_paths = {
            "libavahi-client3": "usr/lib/x86_64-linux-gnu/libavahi-client.so.3",
        },
        deps = [
            "@org_debian_ftp_libavahi_common3//:lib",
            "@org_debian_ftp_libdbus//:lib",
        ],
    )

    tvsc_archive(
        name = "org_debian_ftp_libavahi_common_dev",
        libname = "libavahi-common-dev",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-common-dev_0.8-5+deb11u1_arm64.deb",
        ],
        arm64_sha256 = "8d63fd5df465a89d38d2fdf5bcc505972d84aa98a4554d52040122cdefa266c1",
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
        arm64_sha256 = "8df6cf5dcda28652a29daf0872f5cfe98b2f20fcb9fe7a146e714432189c136d",
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
        arm64_sha256 = "74a2ebfb4796323ea710e47c1f25fe09d05d6e3bfb1dd71007776a80f18441ec",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/libu/libusb-1.0/libusb-1.0-0-dev_1.0.26-1_amd64.deb",
        ],
        x86_64_sha256 = "22a952f48cdb59f1d0c476874facc62418eda5b4c30c5a23a17c28781b426c1b",
    )

    tvsc_archive(
        name = "org_debian_ftp_librtlsdr_dev",
        libname = "librtlsdr_dev",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/r/rtl-sdr/librtlsdr-dev_0.6.0-3_arm64.deb",
        ],
        arm64_sha256 = "9788df9810e32fe750f0855324f87b8d0be13cebd2947f4cfc814213c226717d",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/r/rtl-sdr/librtlsdr-dev_0.6.0-3_amd64.deb",
        ],
        x86_64_sha256 = "b15aa87fc834bfc66f3369b3da1936c8d700fe8e0857104e28ebb3c74aba4a11",
        deps = [
            "@org_debian_ftp_libusb_dev//:lib",
        ],
    )

    tvsc_archive(
        name = "org_debian_ftp_libsoapysdr_dev",
        libname = "libsoapysdr_dev",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/s/soapysdr/libsoapysdr-dev_0.7.2-2_arm64.deb",
        ],
        arm64_sha256 = "e91c3ef6b0d49869290d3fed00849fcf0ddaa47f863a0f274b89bca66aeb279c",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/s/soapysdr/libsoapysdr-dev_0.7.2-2_amd64.deb",
        ],
        x86_64_sha256 = "e00c599336e25c7aa71092160c9cbb51664745cbabeb01bbabc3d01c78955786",
        deps = [
            "@org_debian_ftp_libsoapysdr//:lib",
        ],
    )

    tvsc_archive(
        name = "org_debian_ftp_libsoapysdr",
        libname = "libsoapysdr",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/s/soapysdr/libsoapysdr0.7_0.7.2-2_arm64.deb",
        ],
        arm64_library_paths = {
            "libsoapysdr": "usr/lib/aarch64-linux-gnu/libSoapySDR.so.0.7",
        },
        arm64_sha256 = "a89a1429c051b8861e96a967d54314a4b52278a96de6c6a5162afaf1e4cf2cfe",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/s/soapysdr/libsoapysdr0.7_0.7.2-2_amd64.deb",
        ],
        x86_64_library_paths = {
            "libsoapysdr": "usr/lib/x86_64-linux-gnu/libSoapySDR.so.0.7",
        },
        x86_64_sha256 = "89a4161a374cba943e3023dfe9df1846d5ba4920011accec6c172b44b973b26a",
    )

    tvsc_archive(
        name = "org_debian_ftp_libdbus",
        libname = "libdbus",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/d/dbus/libdbus-1-3_1.12.24-0+deb11u1_arm64.deb",
        ],
        arm64_library_paths = {
            "libdbus": "lib/aarch64-linux-gnu/libdbus-1.so.3",
        },
        arm64_sha256 = "adfe6e1cb49a6716967d20117220a597d2b53ae58056f869bbacfc51ed6f0ba2",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/d/dbus/libdbus-1-3_1.12.24-0+deb11u1_amd64.deb",
        ],
        x86_64_library_paths = {
            "libdbus": "lib/x86_64-linux-gnu/libdbus-1.so.3",
        },
        x86_64_sha256 = "dd594737f53de48c3cbe3431b12207c0c3382a48f257a81d5a7e59fcc6d3ace9",
        deps = [
            "@org_debian_ftp_libsystemd0//:lib",
        ],
    )

    tvsc_archive(
        name = "org_debian_ftp_libsystemd0",
        libname = "libsystemd0",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/s/systemd/libsystemd0_241-7~deb10u8_arm64.deb",
        ],
        arm64_library_paths = {
            "libsystemd0": "lib/aarch64-linux-gnu/libsystemd.so.0",
        },
        arm64_sha256 = "13d2b97bbabd76b99df815c9e73c3502ab9081fa3bb885eae59370b0e290efd5",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/s/systemd/libsystemd0_241-7~deb10u8_amd64.deb",
        ],
        x86_64_library_paths = {
            "libsystemd0": "lib/x86_64-linux-gnu/libsystemd.so.0",
        },
        x86_64_sha256 = "fadce8dbc36955ac93ece6ab2516f927c85480df9419a578c95c388834b4980e",
        deps = [
            "@org_debian_ftp_libcap2//:lib",
            "@org_debian_ftp_libgcrypt//:lib",
            "@org_debian_ftp_liblz4//:lib",
            "@org_debian_ftp_liblzma//:lib",
            "@org_debian_ftp_libzstd//:lib",
        ],
    )

    tvsc_archive(
        name = "org_debian_ftp_libcap2",
        libname = "libcap2",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/libc/libcap2/libcap2_2.44-1_arm64.deb",
        ],
        arm64_library_paths = {
            "libcap": "lib/aarch64-linux-gnu/libcap.so.2",
        },
        arm64_sha256 = "7c5729a1cfd14876685217c5f0545301e7ff1b839262fb487d6a778e8cd8c05a",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/libc/libcap2/libcap2_2.44-1_amd64.deb",
        ],
        x86_64_library_paths = {
            "libcap": "lib/x86_64-linux-gnu/libcap.so.2",
        },
        x86_64_sha256 = "7a3ae3e97d0d403a4c54663c0bb48e9341d98822420a4ab808c6dc8e8474558f",
    )

    tvsc_archive(
        name = "org_debian_ftp_liblz4",
        libname = "liblz4",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/l/lz4/liblz4-1_1.9.4-1_arm64.deb",
        ],
        arm64_library_paths = {
            "liblz4": "usr/lib/aarch64-linux-gnu/liblz4.so.1",
        },
        arm64_sha256 = "f061216ce11aabba8f032dfd6c75c181e782fef7493033b9621a8c3b2953b87e",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/l/lz4/liblz4-1_1.9.4-1_amd64.deb",
        ],
        x86_64_library_paths = {
            "liblz4": "usr/lib/x86_64-linux-gnu/liblz4.so.1",
        },
        x86_64_sha256 = "64cde86cef1deaf828bd60297839b59710b5cd8dc50efd4f12643caaee9389d3",
    )

    tvsc_archive(
        name = "org_debian_ftp_liblzma",
        libname = "liblzma",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/x/xz-utils/liblzma5_5.2.4-1+deb10u1_arm64.deb",
        ],
        arm64_library_paths = {
            "liblzma": "lib/aarch64-linux-gnu/liblzma.so.5",
        },
        arm64_sha256 = "dc81fe6c195f0d2498b2043bbc1443d881d7196dbc445f052dd58f574b3ab95a",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/x/xz-utils/liblzma5_5.2.4-1+deb10u1_amd64.deb",
        ],
        x86_64_library_paths = {
            "liblzma": "lib/x86_64-linux-gnu/liblzma.so.5",
        },
        x86_64_sha256 = "c054750abd5b2c5b2b023329d04e4a8b432df11cd4a64bf842478a4b60a8e140",
    )

    tvsc_archive(
        name = "org_debian_ftp_libgcrypt",
        libname = "libgcrypt",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/libg/libgcrypt20/libgcrypt20_1.8.4-5+deb10u1_arm64.deb",
        ],
        arm64_library_paths = {
            "libgcrypt": "lib/aarch64-linux-gnu/libgcrypt.so.20",
        },
        arm64_sha256 = "189ee02caba07d348840f016c141c51557fcc933fbe43039cacea8319dc419ad",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/libg/libgcrypt20/libgcrypt20_1.8.4-5+deb10u1_amd64.deb",
        ],
        x86_64_library_paths = {
            "libgcrypt": "lib/x86_64-linux-gnu/libgcrypt.so.20",
        },
        x86_64_sha256 = "b29220a4042423b5466869c27bc4b10115e2e3a4c43eda80569b7a98ab35af93",
        deps = [
            "@org_debian_ftp_libgpg-error//:lib",
        ],
    )

    tvsc_archive(
        name = "org_debian_ftp_libzstd",
        libname = "libzstd",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/libz/libzstd/libzstd1_1.5.2+dfsg-1_arm64.deb",
        ],
        arm64_library_paths = {
            "libzstd": "usr/lib/aarch64-linux-gnu/libzstd.so.1",
        },
        arm64_sha256 = "b429bc806ec8cf11fafd35dd40e385b95cce9287efa7b89fa5d7d12f38883d3c",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/libz/libzstd/libzstd1_1.5.2+dfsg-1_amd64.deb",
        ],
        x86_64_library_paths = {
            "libzstd": "usr/lib/x86_64-linux-gnu/libzstd.so.1",
        },
        x86_64_sha256 = "4914489233dbccf83139ee8bff065915982481aa44f3ffcde07a633db5908935",
    )

    tvsc_archive(
        name = "org_debian_ftp_libgpg-error",
        libname = "libgpg-error",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/libg/libgpg-error/libgpg-error0_1.35-1_arm64.deb",
        ],
        arm64_library_paths = {
            "libgpg-error": "lib/aarch64-linux-gnu/libgpg-error.so.0",
        },
        arm64_sha256 = "94dd06fac945a74eecdbaff67dec72e4261d995e7519814c569fba424cdca508",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/libg/libgpg-error/libgpg-error0_1.35-1_amd64.deb",
        ],
        x86_64_library_paths = {
            "libgpg-error": "lib/x86_64-linux-gnu/libgpg-error.so.0",
        },
        x86_64_sha256 = "996b67baf6b5c6fda0db2df27cce15701b122403d0a7f30e9a1f50d07205450a",
    )
