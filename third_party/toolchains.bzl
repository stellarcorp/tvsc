load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def load_toolchains():
    # TODO(james): Lock this to a release tag.
    TOOLCHAINS_COMMIT = "92bf76ddd8a6d13db71b305c9b0b446857fb2135"
    if "com_gitlab_tvsc_toolchains" not in native.existing_rules():
        http_archive(
            name = "com_gitlab_tvsc_toolchains",
            sha256 = "f4157cd0b3be60ee1dedaecf7e2d4145c700c3f7f0aa87f1bac4955a47b7fa20",
            urls = ["https://gitlab.com/tvsc/toolchains/-/archive/{}.zip".format(TOOLCHAINS_COMMIT)],
            strip_prefix = "toolchains-{}".format(TOOLCHAINS_COMMIT),
        )
