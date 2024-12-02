load("//third_party:source_deps.bzl", "load_source_dependencies")
load("//third_party:toolchains.bzl", "load_toolchains")

def load_dependencies():
    load_toolchains()
    load_source_dependencies()
