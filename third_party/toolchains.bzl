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

    # Teensy toolchain URLs were extracted from a JSON file used to configure Arduino libraries and
    # tools: https://www.pjrc.com/teensy/package_teensy_index.json
    #
    # The specific URLs were based on the Teensy avr version 1.59.
    #
    # This package should be kept in sync with the Teensy "package" downloaded in source_deps.bzl.
    #
    # TODO(james): Consider parsing that JSON file automatically, since there is a very good chance
    # that file will change and the URLs for these tools will become invalid.
    if "com_pjrc_teensy_compile" not in native.existing_rules():
        http_archive(
            name = "com_pjrc_teensy_compile",
            sha256 = "9553a7d83b251717c94eb43318de6ca997cb461b19bc73a8c6ca10e9b83cccd4",
            urls = [
                "https://www.pjrc.com/teensy/td_158/teensy-compile-11.3.1-linux64.tar.zst",
            ],
            build_file = "//third_party/teensy:teensy_compile.BUILD",
            patches = [
                "//third_party/teensy:newlib_version.patch",
                "//third_party/teensy:endian_header.patch",
            ],
        )
