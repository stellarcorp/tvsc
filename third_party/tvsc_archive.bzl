load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def _build_arch_targets(*, arch, libname, library_paths = {}):
    library_import_content = """
cc_import(
    name = "{libname}",
    shared_library = "{library_path}",
)
"""

    build_file_content = """        
cc_library(
    name = "conf",
    hdrs = glob(["usr/include/{arch}-linux-gnu/**/*.h", "usr/include/{arch}-linux-gnu/**/*.hpp"]),
    strip_include_prefix = "usr/include/{arch}-linux-gnu",
)

cc_library(
    name = "lib",
    hdrs = glob(["usr/include/**/*.h", "usr/include/**/*.hpp"]),
    strip_include_prefix = "usr/include",
    visibility = ["//visibility:public"],
    deps = [
        ":conf",
        {lib_import_targets}
    ],
)
"""
    result = ""
    lib_import_targets = ""
    for libname, library_path in library_paths.items():
        result += library_import_content.format(libname = libname, library_path = library_path)
        lib_import_targets += '":{libname}", '.format(libname = libname)

    result += build_file_content.format(arch = arch, lib_import_targets = lib_import_targets)

    return result

def tvsc_archive(*, name, libname, x86_64_urls, x86_64_library_paths = {}, x86_64_build_file = "", x86_64_sha256 = "", arm64_urls, arm64_library_paths = {}, arm64_build_file = "", arm64_sha256 = "", deps = []):
    if not native.existing_rule(name):
        x86_64_name = "{}_x86_64".format(name)
        arm64_name = "{}_arm64".format(name)

        if not libname.startswith("lib"):
            libname = "lib" + libname

        if x86_64_build_file:
            http_archive(
                name = x86_64_name,
                urls = x86_64_urls,
                build_file = x86_64_build_file,
                sha256 = x86_64_sha256,
                patch_cmds = ["tar xf data.tar.xz"],
            )
        else:
            x86_64_build_file_content = _build_arch_targets(arch = "x86_64", libname = libname, library_paths = x86_64_library_paths)

            http_archive(
                name = x86_64_name,
                urls = x86_64_urls,
                build_file_content = x86_64_build_file_content,
                sha256 = x86_64_sha256,
                patch_cmds = ["tar xf data.tar.xz"],
            )

        if arm64_build_file:
            http_archive(
                name = arm64_name,
                urls = arm64_urls,
                build_file = arm64_build_file,
                sha256 = arm64_sha256,
                patch_cmds = ["tar xf data.tar.xz"],
            )
        else:
            arm64_build_file_content = _build_arch_targets(arch = "aarch64", libname = libname, library_paths = arm64_library_paths)

            http_archive(
                name = arm64_name,
                urls = arm64_urls,
                build_file_content = arm64_build_file_content,
                sha256 = arm64_sha256,
                patch_cmds = ["tar xf data.tar.xz"],
            )

        # Set up the repo that pivots between the different platforms.
        pivot_repo_build_file_content = """
cc_library(
    name = "lib",
    visibility = ["//visibility:public"],
    deps = select({{
      "@platforms//cpu:arm64": ["@{arm64_name}//:lib"],
      "@platforms//cpu:x86_64": ["@{x86_64_name}//:lib"],
    }}) + {extra_deps},
    target_compatible_with = select({{
      "@platforms//os:none": [
        "@platforms//:incompatible",
      ],
      "//conditions:default": [],
    }}),
)
"""

        extra_deps = "["
        for dep in deps:
            extra_deps += '"{}", '.format(dep)
        extra_deps += "]"
        pivot_repo_build_file_content = pivot_repo_build_file_content.format(name = name, arm64_name = arm64_name, x86_64_name = x86_64_name, extra_deps = extra_deps)

        native.new_local_repository(
            name = name,
            path = "./third_party/empty",
            build_file_content = pivot_repo_build_file_content,
        )
