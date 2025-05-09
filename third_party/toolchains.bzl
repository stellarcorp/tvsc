load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def load_toolchains():
    if "com_github_tvsc_arm-none-eabi-gcc-11.3.1" not in native.existing_rules():
        http_archive(
            name = "com_github_tvsc_arm-none-eabi-gcc-11.3.1",
            sha256 = "8470c924f6f4ea7a25d749339b29ffdb7fd0bf7bfc797a42fc0f6c26123bbb35",
            urls = [
                "https://github.com/stellarcorp/arm-none-eabi-gcc-11.3.1/archive/4cb8d0a60a1a22e7585ca0ba9bd74d3422c0922e.tar.gz",
            ],
            strip_prefix = "arm-none-eabi-gcc-11.3.1-4cb8d0a60a1a22e7585ca0ba9bd74d3422c0922e",
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
