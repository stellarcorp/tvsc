# TVSC

## Getting started

To make it easy for you to get started with GitLab, here's a list of recommended next steps.

Already a pro? Just edit this README.md and make it your own. Want to make it easy? [Use the template at the bottom](#editing-this-readme)!

### Setup Bazelisk
```
wget https://github.com/bazelbuild/bazelisk/releases/download/v1.6.1/bazelisk-linux-amd64
mv bazelisk-linux-amd64 ~/bin/bazel
chmod +x ~/bin/bazel
```

### Verify that ~/bin/ is in your PATH:
```
echo $PATH
```

### Do a test build for your local system:
```
bazel run //:hello_world
```

You can guess what this should print.

Any issues are likely due to missing or out-of-date compilers.

### Do a test build using one of the cross-compilers:
```
bazel build --config=rpi3 //:hello_world
```

This step should succeed, though it make take a while since it will download the toolchain for the Raspberry Pi 3.

This next step should fail, unless you are running on a Raspberry Pi 3:

```
./bazel-bin/hello_world
```
