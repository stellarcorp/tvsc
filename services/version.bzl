load("//platforms:platform.bzl", "arch_name")

VERSION_MAJOR = "0"
VERSION_MINOR = "0"
VERSION_PATCH = "3"
VERSION_STRING = VERSION_MAJOR + "." + VERSION_MINOR + "." + VERSION_PATCH


def version_string():
    return VERSION_STRING


def tarball_name(package_name):
    return package_name + "-" + VERSION_STRING + "-" + arch_name() + ".tgz"
