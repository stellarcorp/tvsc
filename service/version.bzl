load("//platforms:platform.bzl", "arch_name")

VERSION_MAJOR = "0"
VERSION_MINOR = "1"
VERSION_PATCH = "0"
VERSION_STRING = VERSION_MAJOR + "." + VERSION_MINOR + "." + VERSION_PATCH


def version_string():
    return VERSION_STRING


def tarball_name(package_name):
    return package_name + "-" + VERSION_STRING + "-" + arch_name() + ".tgz"


def create_service_dependency_string(service_name):
    return "{} (>= {})".format(service_name, version_string())
