load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

URL_TOOLCHAIN = "https://github.com/ltekieli/devboards-toolchains/releases/download/v2021.12.01/"
URL_SYSROOT = "https://github.com/ltekieli/buildroot/releases/download/v2021.12.01/"

def toolchains():
    if "aarch64-rpi3-linux-gnu" not in native.existing_rules():
        http_archive(
            name = "aarch64-rpi3-linux-gnu",
            build_file = Label("//third_party/toolchains:aarch64-rpi3-linux-gnu.BUILD"),
            url = URL_TOOLCHAIN + "aarch64-rpi3-linux-gnu.tar.gz",
            sha256 = "02d1186d61dd253a6138d858f63c34f4036a80d0b0eeeb8f2d6666c987cadfad",
        )

    if "aarch64-rpi3-linux-gnu-sysroot" not in native.existing_rules():
        http_archive(
            name = "aarch64-rpi3-linux-gnu-sysroot",
            build_file = Label("//third_party/toolchains:aarch64-rpi3-linux-gnu-sysroot.BUILD"),
            url = URL_SYSROOT + "raspberrypi3_64.tar.gz",
            sha256 = "bde52d18418dfa294f7c3442deaaebe644cb1c6e5bb24435255d311f994c06e1",
        )

    if "arm-cortex_a8-linux-gnueabihf" not in native.existing_rules():
        http_archive(
            name = "arm-cortex_a8-linux-gnueabihf",
            build_file = Label("//third_party/toolchains:arm-cortex_a8-linux-gnueabihf.BUILD"),
            url = URL_TOOLCHAIN + "arm-cortex_a8-linux-gnueabihf.tar.gz",
            sha256 = "bf731dcdc1b8e925334a26d01d822f5e65efc580785f9fcbf90405b9397f16a8",
        )

    if "arm-cortex_a8-linux-gnueabihf-sysroot" not in native.existing_rules():
        http_archive(
            name = "arm-cortex_a8-linux-gnueabihf-sysroot",
            build_file = Label("//third_party/toolchains:arm-cortex_a8-linux-gnueabihf-sysroot.BUILD"),
            url = URL_SYSROOT + "beaglebone.tar.gz",
            sha256 = "c716440776ec0e8823e226268ce490bf7e705c2c869e41b1bebcf26ff99fd19d",
        )
