load("//third_party:debian_deps.bzl", "load_debian_dependencies")
load("//third_party:source_deps.bzl", "load_source_dependencies")
load("//third_party:toolchains.bzl", "load_toolchains")

def load_dependencies():
    load_toolchains()
    load_debian_dependencies()
    load_source_dependencies()
