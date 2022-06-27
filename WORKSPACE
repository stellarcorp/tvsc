load("//third_party:deps.bzl", "load_dependencies")
load_dependencies()

load("//toolchain:toolchain.bzl", "register_all_toolchains")
register_all_toolchains()

load(
    "@io_bazel_rules_docker//repositories:repositories.bzl",
    container_repositories = "repositories",
)

container_repositories()

load(
    "@io_bazel_rules_docker//cc:image.bzl",
    _cc_image_repos = "repositories",
)

_cc_image_repos()
