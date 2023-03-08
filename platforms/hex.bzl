load("@rules_cc//cc:find_cc_toolchain.bzl", "find_cc_toolchain")

def _hex_impl(ctx):
    cc_toolchain = find_cc_toolchain(ctx)
    outfile = ctx.outputs.out
    infile = ctx.file.src

    ctx.actions.run_shell(
        outputs = [outfile],
        inputs = depset(
            direct = [ctx.file.src],
            transitive = [
                cc_toolchain.all_files,
            ],
        ),
        command = "{objcopy} -O ihex -R .eeprom {src} {dst}".format(
            objcopy = cc_toolchain.objcopy_executable,
            src = ctx.file.src.path,
            dst = ctx.outputs.out.path,
        ),
    )

    return [
        DefaultInfo(
            files = depset([outfile]),
        ),
    ]

hex = rule(
    implementation = _hex_impl,
    attrs = {
        "src": attr.label(allow_single_file = True, mandatory = True),
        "out": attr.output(mandatory = True),
    },
    executable = False,
    toolchains = ["@bazel_tools//tools/cpp:toolchain_type"],
    fragments = ["cpp"],
)
