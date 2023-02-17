load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")
load(
    "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "feature",
    "feature_set",
    "flag_group",
    "flag_set",
    "tool_path",
    "with_feature_set",
)

all_link_actions = [
    ACTION_NAMES.cpp_link_executable,
    ACTION_NAMES.cpp_link_dynamic_library,
    ACTION_NAMES.cpp_link_nodeps_dynamic_library,
]

all_compile_actions = [
    ACTION_NAMES.assemble,
    ACTION_NAMES.c_compile,
    ACTION_NAMES.clif_match,
    ACTION_NAMES.cpp_compile,
    ACTION_NAMES.cpp_header_parsing,
    ACTION_NAMES.cpp_module_codegen,
    ACTION_NAMES.cpp_module_compile,
    ACTION_NAMES.linkstamp_compile,
    ACTION_NAMES.lto_backend,
    ACTION_NAMES.preprocess_assemble,
]

def _impl(ctx):
    tool_paths = [
        tool_path(
            name = "ar",
            path = "wrappers/aarch64-none-linux-gnu-ar",
        ),
        tool_path(
            name = "cpp",
            path = "wrappers/aarch64-none-linux-gnu-cpp",
        ),
        tool_path(
            name = "gcc",
            path = "wrappers/aarch64-none-linux-gnu-gcc",
        ),
        tool_path(
            name = "gcov",
            path = "wrappers/aarch64-none-linux-gnu-gcov",
        ),
        tool_path(
            name = "ld",
            path = "wrappers/aarch64-none-linux-gnu-ld",
        ),
        tool_path(
            name = "nm",
            path = "wrappers/aarch64-none-linux-gnu-nm",
        ),
        tool_path(
            name = "objdump",
            path = "wrappers/aarch64-none-linux-gnu-objdump",
        ),
        tool_path(
            name = "strip",
            path = "wrappers/aarch64-none-linux-gnu-strip",
        ),
    ]

    dbg_feature = feature(name = "dbg")

    opt_feature = feature(name = "opt")

    shared_library_feature = feature(
        name = "supports_dynamic_linker",
        enabled = True,
    )

    default_compiler_flags = feature(
        name = "default_compiler_flags",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = all_compile_actions,
                flag_groups = [
                    flag_group(
                        flags = [
                            "-no-canonical-prefixes",
                            "-fno-canonical-system-headers",
                            "-Wno-builtin-macro-redefined",
                            "-D__DATE__=\"redacted\"",
                            "-D__TIMESTAMP__=\"redacted\"",
                            "-D__TIME__=\"redacted\"",
                            "-fPIC",
                            "-U_FORTIFY_SOURCE",
                        ],
                    ),
                ],
            ),
        ],
    )

    opt_compiler_flags = feature(
        name = "opt_compiler_flags",
        enabled = True,
        requires = [feature_set(features = ["opt"])],
        flag_sets = [
            flag_set(
                actions = all_compile_actions,
                flag_groups = [
                    flag_group(
                        flags = [
                            "-O2",
                        ],
                    ),
                ],
            ),
        ],
    )

    dbg_compiler_flags = feature(
        name = "dbg_compiler_flags",
        enabled = True,
        requires = [feature_set(features = ["dbg"])],
        flag_sets = [
            flag_set(
                actions = all_compile_actions,
                flag_groups = [
                    flag_group(
                        flags = [
                            "-g",
                            "-O",
                        ],
                    ),
                ],
            ),
        ],
    )

    toolchain_debug_linker_flags = feature(
        name = "toolchain_debug_linker_flags",
        enabled = False,
        flag_sets = [
            flag_set(
                actions = all_link_actions,
                flag_groups = ([
                    flag_group(
                        flags = [
                            "-Wl,--verbose",
                        ],
                    ),
                ]),
            ),
        ],
    )

    default_linker_flags = feature(
        name = "default_linker_flags",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = all_link_actions,
                flag_groups = ([
                    flag_group(
                        flags = [
                            "-lstdc++",
                            "-lm",
                        ],
                    ),
                ]),
            ),
        ],
    )

    features = [
        opt_feature,
        dbg_feature,
        opt_compiler_flags,
        dbg_compiler_flags,
        shared_library_feature,
        default_compiler_flags,
        default_linker_flags,
        toolchain_debug_linker_flags,
    ]

    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features,
        toolchain_identifier = "arm64-debian-toolchain",
        host_system_name = "local",
        target_system_name = "unknown",
        target_cpu = "unknown",
        target_libc = "unknown",
        compiler = "unknown",
        abi_version = "unknown",
        abi_libc_version = "unknown",
        tool_paths = tool_paths,
        builtin_sysroot = "external/com_gitlab_tvsc_gcc_arm_10_2/gcc-arm-10.2-2020.11-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc",
    )

cc_toolchain_config = rule(
    implementation = _impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)
