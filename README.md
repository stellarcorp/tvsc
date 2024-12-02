# TVSC

This repository is the principal codebase for the Tennessee Valley Stellar Corporation. We are a community-scale space agency in east Tennessee dedicated to the open development of technologies enabling the public use of space. Our first project is a drone-based communication system called "Virtual Tower". Its goal is to extend radio communications without the need of a physical radio tower. The Virtual Tower is currently under development and is not yet ready for general use.

## Repository structure

As the Virtual Tower progresses, we will add specific directories to this repository for that project. Until then, the best places to start are the [radio](./radio) and the [service](./service) directories. The radio directory contains APIs for working with low-level radio modules, often on embedded systems. The service directory contains the various services that will comprise the Virtual Tower and future projects.

## Getting started

### Checkout this repo

See instructions from gitlab.com.

### Bazel

We use Bazel for our build system. It is one of the few build tools that allow for multi-language, cross-platform hermetic builds. For more information, checkout https://bazel.build/

Part of Bazel is a command line tool called [Bazelisk](https://github.com/bazelbuild/bazelisk). This tool manages Bazel installations on your local machine and allows us to specify *in the repo itself* the version of Bazel to use and prevents build errors due to build system versioning. Bazelisk executables are checked into the top-level of this repo. Please use them when building.

On the first run of bazelisk on a given computer, it will download the correct version of Bazel. So, this first build may take a while.

### Do a test build for your local system:

From the top-level of the repo,
```
./bazelisk-linux-amd64 run //service/hello:hello_world
```

You can guess what this should print. Any issues are likely due to missing or out-of-date compilers.

Note: future examples will use `bazel` instead of one of the bazelisk executables. This is inline with examples from the [Bazel documentation](https://bazel.build/) and other online sources. Please continue to use bazelisk, even when the examples say `bazel`. You may find it useful to make an alias from `bazel` to the correct bazelisk executable.
