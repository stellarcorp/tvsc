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
            path = "wrappers/arm-none-eabi-ar",
        ),
        tool_path(
            name = "cpp",
            path = "wrappers/arm-none-eabi-cpp",
        ),
        tool_path(
            name = "gcc",
            path = "wrappers/arm-none-eabi-gcc",
        ),
        tool_path(
            name = "gcov",
            path = "wrappers/arm-none-eabi-gcov",
        ),
        tool_path(
            name = "ld",
            path = "wrappers/arm-none-eabi-ld",
        ),
        tool_path(
            name = "nm",
            path = "wrappers/arm-none-eabi-nm",
        ),
        tool_path(
            name = "objcopy",
            path = "wrappers/arm-none-eabi-objcopy",
        ),
        tool_path(
            name = "objdump",
            path = "wrappers/arm-none-eabi-objdump",
        ),
        tool_path(
            name = "strip",
            path = "wrappers/arm-none-eabi-strip",
        ),
    ]

    dbg_feature = feature(name = "dbg")
    fastbuild_feature = feature(name = "fastbuild")
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
                            "-Wall",
                            "-ffunction-sections",
                            "-fdata-sections",
                            "-nostdlib",
                            "-fno-exceptions",
                            "-fpermissive",
                            "-fno-rtti",
                            "-fno-threadsafe-statics",
                            "-felide-constructors",
                            "-Wno-error=narrowing",
                            "-mthumb",
                            "-mcpu=cortex-m7",
                            "-mfloat-abi=hard",
                            "-mfpu=fpv5-d16",
                            "-D__IMXRT1062__",
                            "-DLAYOUT_US_ENGLISH",
                            "-DTEENSYDUINO=159",
                            "-D_DEFAULT_SOURCE=1",
                            "-DARDUINO=10607",
                            "-DARDUINO_TEENSY40",
                            "-DUSB_SERIAL",
                            "-no-canonical-prefixes",
                            "-fno-canonical-system-headers",
                            "-Wno-builtin-macro-redefined",
                            "-D__DATE__=\"redacted\"",
                            "-D__TIMESTAMP__=\"redacted\"",
                            "-D__TIME__=\"redacted\"",
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
                            "-Os",
                        ],
                    ),
                ],
            ),
        ],
    )

    fastbuild_compiler_flags = feature(
        name = "fastbuild_compiler_flags",
        enabled = True,
        requires = [feature_set(features = ["fastbuild"])],
        flag_sets = [
            flag_set(
                actions = all_compile_actions,
                flag_groups = [
                    flag_group(
                        flags = [
                            "-O",
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
                            "--specs=nano.specs",
                            "-Wl,--gc-sections,--relax",
                            "-mthumb",
                            "-mcpu=cortex-m7",
                            "-mfloat-abi=hard",
                            "-mfpu=fpv5-d16",
                            "-larm_cortexM7lfsp_math",
                            "-lstdc++",
                            "-lm",
                            "-Texternal/com_pjrc_teensy_package/avr/cores/teensy4/imxrt1062.ld",
                        ],
                    ),
                ]),
            ),
        ],
    )

    features = [
        opt_feature,
        fastbuild_feature,
        dbg_feature,
        opt_compiler_flags,
        fastbuild_compiler_flags,
        dbg_compiler_flags,
        shared_library_feature,
        default_compiler_flags,
        default_linker_flags,
        toolchain_debug_linker_flags,
    ]

    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features,
        toolchain_identifier = "armv7e-mf-toolchain",
        host_system_name = "local",
        target_system_name = "arm-none-eabi-gcc",
        target_cpu = "armv7e-mf",
        target_libc = "unknown",
        compiler = "arm-none-eabi-gcc",
        abi_version = "unknown",
        abi_libc_version = "unknown",
        tool_paths = tool_paths,
        builtin_sysroot = "external/com_pjrc_teensy_compile/tools/arm/arm-none-eabi",
    )

cc_toolchain_config = rule(
    implementation = _impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)
