#TVSC

This repository is the principal codebase for the Tennessee Valley Stellar Corporation. We are a community-scale space agency in east Tennessee dedicated to the open development of technologies enabling the public use of space.

## Repository structure

There are three main sections to the repository. The most active development at the moment is taking place in the [hal](./hal) and the [electronics](./electronics) directories. The hal directory contains the Hardware Abstraction Layer. The electronics directory holds our PCB design work. The rest of the repository (the third section) consists of older code that is in need of maintenance/documentation or deletion.

## Getting started

### Bazel

We use Bazel for our build system. It is one of the few build tools that allow for multi-language, cross-platform hermetic builds. For more information, checkout https://bazel.build/

Part of Bazel is a command line tool called [Bazelisk](https://github.com/bazelbuild/bazelisk). This tool manages Bazel installations on your local machine and allows us to specify *in the repo itself* the version of Bazel to use and prevents build errors due to build system versioning. Bazelisk executables are checked into the top-level of this repo. Please use them when building.

On the first run of bazelisk on a given computer, it will download the correct version of Bazel. So, this first build may take a while.

### Toolchain

The build uses the local toolchain for desktop builds. This toolchain should support C++20.

For embedded builds, the toolchain and system libraries are downloaded from various external repositories.

The builds are designed to work on Linux. Other systems can be added as needed.

### Do a test build for your local system:

From the top-level of the repo,
```
./bazelisk-linux-amd64 build --config=nucleo_l452re //...
```

The first build will take a long time, but future builds should be relatively fast.
