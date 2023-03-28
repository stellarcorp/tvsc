load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("//third_party:tvsc_archive.bzl", "tvsc_archive")

# Load Debian packages as prebuilt dependencies. The exact packages should come from
# https://ftp.debian.org/debian/pool/main, and ideally, we use the exact same version for every
# platform.
# We base the package versions around those shipped/tagged for Debian 10 (Buster), picking the most
# recent update within Debian 10. Usually, these packages are tagged as deb10u1 (or deb10u2 for the
# 2nd update, etc.).
# TODO(james): Audit the versions chosen for each package to verify that it is a Debian 10 package.
def load_debian_dependencies():
    # zstd
    tvsc_archive(
        name = "org_debian_ftp_libzstd_dev",
        libname = "libzstd-dev",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/libz/libzstd/libzstd-dev_1.3.8+dfsg-3+deb10u2_arm64.deb",
        ],
        arm64_sha256 = "40f0db5735af5bb6b75bc76be4fde2f51d577d90f9e69ea13376d15a2d80ba01",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/libz/libzstd/libzstd-dev_1.3.8+dfsg-3+deb10u2_amd64.deb",
        ],
        x86_64_sha256 = "4204e768783f2e16b8bad455a564e8bed28053f59c7f49786d341a4920948f70",
    )

    # OpenSSL
    tvsc_archive(
        name = "org_debian_ftp_libssl_dev",
        libname = "libssl-dev",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/o/openssl/libssl-dev_1.1.1n-0+deb10u3_arm64.deb",
        ],
        arm64_sha256 = "6021a64426ce819de704a662b69b60c7a15889e93f63bcbf5cb3c9c26c5dde5d",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/o/openssl/libssl-dev_1.1.1n-0+deb10u3_amd64.deb",
        ],
        x86_64_sha256 = "9414e95c2bfbc5b3252b832da7ffdeefaf014ae7cd72902353077c35fdaa6e68",
        deps = ["@org_debian_ftp_libssl//:lib"],
    )

    tvsc_archive(
        name = "org_debian_ftp_libssl",
        libname = "libssl1.1",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/o/openssl/libssl1.1_1.1.1n-0+deb10u3_arm64.deb",
        ],
        arm64_library_paths = {
            "libssl": "usr/lib/aarch64-linux-gnu/libssl.so.1.1",
            "libcrypto": "usr/lib/aarch64-linux-gnu/libcrypto.so.1.1",
        },
        arm64_sha256 = "9d6db56bf17ec079675e0f6f7772af65b832dd3ad55c02c5409462cf6b052e23",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/o/openssl/libssl1.1_1.1.1n-0+deb10u3_amd64.deb",
        ],
        x86_64_library_paths = {
            "libssl": "usr/lib/x86_64-linux-gnu/libssl.so.1.1",
            "libcrypto": "usr/lib/x86_64-linux-gnu/libcrypto.so.1.1",
        },
        x86_64_sha256 = "92247c2c011ec4c01f5832a38de0ec1a9c2cfac5c6c5dc229e5a1ac81148854c",
    )

    # WiringPi
    tvsc_archive(
        name = "com_ubuntu_ports_libwiringpi_dev",
        libname = "libwiringpi-dev",
        arm64_urls = [
            "http://us.ports.ubuntu.com/ubuntu-ports/pool/universe/w/wiringpi/libwiringpi-dev_2.50-0ubuntu1_arm64.deb",
        ],
        arm64_sha256 = "29e3928b26cbb3aea4db727bd96e0df7a28f7f7a53fbaf01b6f643d4792a2034",
        arm64_patches = ["//third_party/arduino/wiring_pi:arduino_h.patch"],
        x86_64_urls = [
            "http://mirrors.kernel.org/ubuntu/pool/universe/w/wiringpi/libwiringpi-dev_2.50-0ubuntu1_amd64.deb",
        ],
        x86_64_sha256 = "168b0bcc6a5e148f1c9e4280f854cc89469829d25df7e75a084b789be0edf8b1",
        x86_64_patches = ["//third_party/arduino/wiring_pi:arduino_h.patch"],
    )

    tvsc_archive(
        name = "com_ubuntu_ports_libwiringpi",
        libname = "libwiringpi",
        arm64_urls = [
            "http://us.ports.ubuntu.com/ubuntu-ports/pool/universe/w/wiringpi/libwiringpi2_2.50-0ubuntu1_arm64.deb",
        ],
        arm64_library_paths = {
            "libwiringPiDev": "usr/lib/aarch64-linux-gnu/libwiringPiDev.so.2",
            "libwiringPi": "usr/lib/aarch64-linux-gnu/libwiringPi.so.2",
        },
        arm64_sha256 = "b81a03902ec3ebdb88d28080b82b0b373680fcd2e26659240c78716e897e27ac",
        x86_64_urls = [
            "http://mirrors.kernel.org/ubuntu/pool/universe/w/wiringpi/libwiringpi2_2.50-0ubuntu1_amd64.deb",
        ],
        x86_64_library_paths = {
            "libwiringPiDev": "usr/lib/x86_64-linux-gnu/libwiringPiDev.so.2",
            "libwiringPi": "usr/lib/x86_64-linux-gnu/libwiringPi.so.2",
        },
        x86_64_sha256 = "d8a804b4b1fc94703f8d3141bb9344fc346d0f331b2b5ab1af479a64d7b27fb6",
    )

    tvsc_archive(
        name = "kr_or_linuxfactory_libwiringpi_dev",
        libname = "odroid_libwiringpi-dev",
        arm64_urls = [
            "http://ppa.linuxfactory.or.kr/pool/main/o/odroid-wiringpi/libwiringpi-dev_3.14.6+202303091450~focal_arm64.deb",
        ],
        arm64_sha256 = "84528d6e07172da0a2868fb8c6afc0847291ca81fc58106ba7c9ef909b327d8b",
        arm64_patches = ["//third_party/arduino/wiring_pi:arduino_h.patch"],
        arm64_build_file = "//third_party/arduino/wiring_pi:odroid_libwiringpi_dev.BUILD",
    )

    tvsc_archive(
        name = "kr_or_linuxfactory_libwiringpi",
        libname = "odroid_libwiringpi",
        arm64_urls = [
            "http://ppa.linuxfactory.or.kr/pool/main/o/odroid-wiringpi/libwiringpi2_3.14.6+202303091450~focal_arm64.deb",
        ],
        arm64_library_paths = {
            "libwiringPiDev": "usr/lib/aarch64-linux-gnu/libwiringPiDev.so.2",
            "libwiringPi": "usr/lib/aarch64-linux-gnu/libwiringPi.so.2",
        },
        arm64_sha256 = "784ff422271f1dbbc3ae061a2c973ab234b470b5f9005a3d7e2fd56ee91bb71e",
        deps = [
            "@com_ubuntu_ports_libcrypt1//:lib",
        ],
    )

    # libcrypt1, only needed by libwiringpi2.
    tvsc_archive(
        name = "com_ubuntu_ports_libcrypt1",
        libname = "libcrypt1",
        arm64_urls = [
            "http://us.ports.ubuntu.com/ubuntu-ports/pool/main/libx/libxcrypt/libcrypt1_4.4.10-10ubuntu4_arm64.deb",
        ],
        arm64_library_paths = {
            "libcrypt": "lib/aarch64-linux-gnu/libcrypt.so.1",
        },
        arm64_sha256 = "89907b6c1b613c430a53ef10909934b9ce5854a396cd173360495f6f8e5e7ea4",
    )

    # Avahi
    tvsc_archive(
        name = "org_debian_ftp_libavahi_client_dev",
        libname = "libavahi-client-dev",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-client-dev_0.7-4+deb10u1_arm64.deb",
        ],
        arm64_sha256 = "828a794c9fcdf795413b2ac419e6b21b7717e406fd44a9caf2b09eb4fd38f06b",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-client-dev_0.7-4+deb10u1_amd64.deb",
        ],
        x86_64_sha256 = "f3934c93e758d00794cfa55f2f03a28649bdc9441bacc103c4257cebafd389e6",
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
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-client3_0.7-4+deb10u1_arm64.deb",
        ],
        arm64_library_paths = {
            "libavahi-client3": "usr/lib/aarch64-linux-gnu/libavahi-client.so.3",
        },
        arm64_sha256 = "e98034de7b584b49ec7a9524c533fbb6a67289ed48329258e463d05b0a1d2187",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-client3_0.7-4+deb10u1_amd64.deb",
        ],
        x86_64_sha256 = "fe553e88db5448b19fe3900b4923c7a77cbbb3cfe3f80f94111df65128fa35b9",
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
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-common-dev_0.7-4+deb10u1_arm64.deb",
        ],
        arm64_sha256 = "4f005ffac436ff589a093fa114f61517cae79d2e36086570c1f8a873aec81cb8",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-common-dev_0.7-4+deb10u1_amd64.deb",
        ],
        x86_64_sha256 = "9f0a82543e838de2fd2321c1b3c0638152093ae0926f362b7188462d7a2ebb45",
        deps = [
            "@org_debian_ftp_libavahi_common3//:lib",
        ],
    )

    tvsc_archive(
        name = "org_debian_ftp_libavahi_common3",
        libname = "libavahi-common3",
        arm64_urls = [
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-common3_0.7-4+deb10u1_arm64.deb",
        ],
        arm64_library_paths = {
            "libavahi-common3": "usr/lib/aarch64-linux-gnu/libavahi-common.so.3",
        },
        arm64_sha256 = "9fb59243a90535ec02480cd139e1dca30507c05db246cf32ddd8ac254c68f963",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/a/avahi/libavahi-common3_0.7-4+deb10u1_amd64.deb",
        ],
        x86_64_sha256 = "5a3f333cc0f56054d9003c8e28a116a77be9227110bd648dc05db1b1fb9e48a1",
        x86_64_library_paths = {
            "libavahi-common3": "usr/lib/x86_64-linux-gnu/libavahi-common.so.3",
        },
    )

    # libusb
    # TODO(james): Verify these packages are part of Debian 10.
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

    # RTL-SDR
    # TODO(james): Verify these packages are part of Debian 10.
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

    # SoapySDR
    # TODO(james): Verify these packages are part of Debian 10.
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

    # TODO(james): Verify these packages are part of Debian 10.
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
            "https://ftp.debian.org/debian/pool/main/d/dbus/libdbus-1-3_1.12.20-0+deb10u1_arm64.deb",
        ],
        arm64_library_paths = {
            "libdbus": "lib/aarch64-linux-gnu/libdbus-1.so.3",
        },
        arm64_sha256 = "8d1c11bd21a668894ac9b9a392c10521b77b3ab57b51ec14ca2db7f4f5a45de8",
        x86_64_urls = [
            "https://ftp.debian.org/debian/pool/main/d/dbus/libdbus-1-3_1.12.20-0+deb10u1_amd64.deb",
        ],
        x86_64_library_paths = {
            "libdbus": "lib/x86_64-linux-gnu/libdbus-1.so.3",
        },
        x86_64_sha256 = "e394bd35626e3ccf437e1e7776e6573636e6413b0ebe2483bd54ac243eed1007",
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
        ],
    )

    # TODO(james): Verify these packages are part of Debian 10.
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

    # TODO(james): Verify these packages are part of Debian 10.
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

    # TODO(james): Verify these packages are part of Debian 10.
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

    # TODO(james): Verify these packages are part of Debian 10.
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

    # TODO(james): Verify these packages are part of Debian 10.
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
