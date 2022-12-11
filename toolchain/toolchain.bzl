def register_all_toolchains():
    native.register_toolchains(
        "//toolchain/arm64-rpi3-linux-gnu:arm64_linux_toolchain",
        "//toolchain/arm64-debian-linux-gnu:arm64_linux_toolchain",
    )

