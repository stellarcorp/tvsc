# TVSC

## Getting started

### Checkout this repo

See instructions from gitlab.com.

### Bazel

We use Bazel for our build system. It is one of the few build tools that allow for multi-language, cross-platform hermetic builds. For more information, checkout https://bazel.build/

Part of Bazel is a command line tool called [Bazelisk|https://github.com/bazelbuild/bazelisk]. This tool manages Bazel installations on your local machine and allows us to specify *in the repo itself* the version of Bazel to use and prevents build errors due to build system versioning. Bazelisk executables are checked into the top-level of this repo. Please use them when building. (I have an alias called `bazel` that calls bazelisk. This feels more natural to me, personally.)

On the first run of bazelisk on a given computer, it will download the correct version of Bazel. So, this first build may take a while.

### Do a test build for your local system:

From the top-level of the repo,
```
./bazelisk-linux-amd64 run //services/hello:hello_world
```

You can guess what this should print.

For windows, the equivalent should be something along the lines of:
```
bazelisk-windows-amd64.exe run //services/hello:hello_world
```

Any issues are likely due to missing or out-of-date compilers.

Note: future examples will use `bazel` instead of one of the bazelisk executables. This is inline with examples from the [Bazel documentation|https://bazel.build/] and other online sources. Please continue to use bazelisk, even when the examples say `bazel`. This is one of the primary reasons I make an alias from `bazel` to the correct bazelisk executable.

### Do a test build using one of the cross-compilers:
```
bazel build --config=rpi3_cross //services/hello:hello_world
```

This step should succeed, though it make take a while since it will download the toolchain for the Raspberry Pi 3.

This next step should fail, unless you are running on a Raspberry Pi 3:

```
./bazel-bin/services/hello/hello_world
```
