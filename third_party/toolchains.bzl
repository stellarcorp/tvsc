load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def load_toolchains():
    # TODO(james): Lock this to a release tag.
    GCC_ARM_10_2_TOOLCHAIN_COMMIT = "58165f80e16737a7357c32d6e99a893750b726ad"
    if "com_gitlab_tvsc_gcc_arm_10_2" not in native.existing_rules():
        http_archive(
            name = "com_gitlab_tvsc_gcc_arm_10_2",
            sha256 = "6994c2eab429fbbe125fd521c0f323461a85bf29666d53b17ba3760dcee3d259",
            urls = ["https://gitlab.com/tvsc/gcc-arm-10.2/-/archive/{}.zip".format(GCC_ARM_10_2_TOOLCHAIN_COMMIT)],
            strip_prefix = "gcc-arm-10.2-{}".format(GCC_ARM_10_2_TOOLCHAIN_COMMIT),
        )
