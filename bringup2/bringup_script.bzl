"""Macro to build bringup scripts."""

def bringup_binary(
    name,
    srcs,
    deps = [],
    additional_linker_inputs = ["//platforms:linker_script"],
    linkopts = ["-Wl,-T$(execpath //platforms:linker_script)"],
    target_compatible_with = select({
        "@platforms//os:none": [],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    **kwargs):
  """Defines a standardized cc_binary for embedded targets with boilerplate defaults."""

  default_deps = [
      "//base",
      "//hal2",
      "//hal2/board",
      "//hal2/config",
      "//hal2/gpio",
      "//meta:build_time",
      "//platforms:startup",
  ]

  native.cc_binary(
      name = name,
      srcs = srcs,
      additional_linker_inputs = additional_linker_inputs,
      linkopts = linkopts,
      target_compatible_with = target_compatible_with,
      deps = default_deps + deps,
      **kwargs
  )
