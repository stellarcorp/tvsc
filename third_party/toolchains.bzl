load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def load_toolchains():
    # TODO(james): Lock this to a release tag.
    TOOLCHAINS_COMMIT = "6dbdffc5c9a9c8b570cfdbe0e34decf677aca0ce"
    if "com_gitlab_tvsc_toolchains" not in native.existing_rules():
        http_archive(
            name = "com_gitlab_tvsc_toolchains",
            sha256 = "f7007c2ecae4ed88c4f0504367af00701f17fff43b8cf38d2bfb2ba54268aa1c",
            urls = ["https://gitlab.com/tvsc/toolchains/-/archive/{}.zip".format(TOOLCHAINS_COMMIT)],
            strip_prefix = "toolchains-{}".format(TOOLCHAINS_COMMIT),
        )
