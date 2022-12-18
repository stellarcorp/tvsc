load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def load_toolchains():
    # TODO(james): Lock this to a release tag.
    TOOLCHAINS_COMMIT = "b294fa6855299e7e150dfa71db58d4e24dee74d2"
    if "com_gitlab_tvsc_toolchains" not in native.existing_rules():
        http_archive(
            name = "com_gitlab_tvsc_toolchains",
            sha256 = "be673c9666e11ec5105dfe94526e8b3571056249d4d0a338272df53056ac9611",
            urls = ["https://gitlab.com/tvsc/toolchains/-/archive/{}.zip".format(TOOLCHAINS_COMMIT)],
            strip_prefix = "toolchains-{}".format(TOOLCHAINS_COMMIT),
        )
