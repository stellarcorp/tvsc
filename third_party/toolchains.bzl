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
    # The specific URLs were based on the Teensy avr version 0.58.3. Presumably, this was given
    # the version "0.58.3" as a way of marking it as "beta". The Arduino IDE probably chooses
    # the highest version number by default, 1.57.2 in this case. I expect this version number
    # to become 1.58.x in the near future.
    #
    # This package should be kept in sync with the Teensy "package" downloaded in source_deps.bzl.
    #
    # TODO(james): Consider parsing that JSON file automatically, since there is a very good chance
    # that file will change and the URLs for these tools will become invalid.
    if "com_pjrc_teensy_compile" not in native.existing_rules():
        http_archive(
            name = "com_pjrc_teensy_compile",
            sha256 = "936e53df932c156f8aff869960cd9bf158fae5043a8cf6d335410c2221d73a8e",
            urls = [
                "https://www.pjrc.com/teensy/td_158-beta3/teensy-compile-linux64.tar.bz2",
            ],
            build_file = "//third_party/teensy:teensy_compile.BUILD",
            patches = [
                "//third_party/teensy:newlib_version.patch",
                "//third_party/teensy:endian_header.patch",
            ],
        )
