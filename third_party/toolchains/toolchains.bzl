load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

URL_TOOLCHAIN = "https://github.com/ltekieli/devboards-toolchains/releases/download/v2021.12.01/"
URL_SYSROOT = "https://github.com/ltekieli/buildroot/releases/download/v2021.12.01/"

def toolchains():
    # TODO(james): Lock this to a release tag.
    TOOLCHAINS_COMMIT = "6dbdffc5c9a9c8b570cfdbe0e34decf677aca0ce"
    if "com_gitlab_tvsc_toolchains" not in native.existing_rules():
        http_archive(
            name = "com_gitlab_tvsc_toolchains",
            urls = ["https://gitlab.com/tvsc/toolchains/-/archive/{}.zip".format(TOOLCHAINS_COMMIT)],
            strip_prefix = "toolchains-{}".format(TOOLCHAINS_COMMIT),
        )

    # TODO(james): Rebuild these. See https://gitlab.com/tvsc/tvsc/-/issues/1
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
