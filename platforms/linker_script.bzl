load("@bazel_tools//tools/cpp:toolchain_utils.bzl", "find_cpp_toolchain")

def _preprocess_linker_script_impl(ctx):
    # Retrieve the C++ toolchain for the target platform
    cc_toolchain = find_cpp_toolchain(ctx)

    # Get compiler binary path directly from the toolchain object
    compiler_path = cc_toolchain.compiler_executable

    # Automatically pull ALL --copt flags
    copts = ctx.fragments.cpp.copts

    # Collect headers and include paths
    header_files = ctx.files.hdrs
    include_dirs = depset([f.dirname for f in header_files]).to_list()

    # Build command arguments
    args = ctx.actions.args()
    args.add("-E")
    args.add("-P")
    args.add("-xc")

    # Add in the -copt flags
    args.add_all(copts)

    # Add include paths
    for d in include_dirs:
        args.add("-I" + d)

    args.add(ctx.file.src)
    args.add("-o", ctx.outputs.out)

    ctx.actions.run(
        executable = compiler_path,
        arguments = [args],
        inputs = [ctx.file.src] + header_files,
        tools = [cc_toolchain.all_files],
        outputs = [ctx.outputs.out],
        mnemonic = "PreprocessLinkerScript",
        progress_message = "Preprocessing linker script %s" % ctx.file.src.short_path,
        env = ctx.configuration.default_shell_env,
    )

    return [DefaultInfo(files = depset([ctx.outputs.out]))]

preprocess_linker_script = rule(
    implementation = _preprocess_linker_script_impl,
    attrs = {
        "src": attr.label(allow_single_file = [".S", ".ld.S", ".ld"], mandatory = True),
        "hdrs": attr.label_list(allow_files = True),
        "out": attr.output(mandatory = True),
        "_cc_toolchain": attr.label(default = "@bazel_tools//tools/cpp:current_cc_toolchain"),
    },
    fragments = ["cpp"],
    toolchains = ["@bazel_tools//tools/cpp:toolchain_type"],
)
