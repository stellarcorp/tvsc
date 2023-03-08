# Supported Platforms

This package defines the support platforms and the features that differentiate them. These defintions allow us to build each product for each platform. It is based on Bazel's [platform build](https://bazel.build/extending/platforms) features.

## Constraint settings and values

The main design elements at work here are the constraint settings and values. The constraint settings give a category, such as operating system. The constraint values tell the value the platform has for that setting (such as "Linux"). Not all platforms give values for all settings; they only give values for the settings that are relevant to that platform. Then, the build targets themselves indicate which values are required to build that target using the [`target_compatible_with`](https://bazel.build/reference/be/common-definitions#common.target_compatible_with) selector.

The overall effect is that all of the code for each SKU can be built the same way: `bazel build --config <configuration> //...` where the `configuration` indicates which platform is being built.
