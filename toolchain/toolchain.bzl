def register_all_toolchains():
    native.register_toolchains(
        "//toolchain/arm64-debian-linux-gnu:arm64_linux_toolchain",
        "//toolchain/nucleo_l4:stm32l4xx-armv7e-mf_none_toolchain",
        "//toolchain/teensy:armv7e-mf_none_toolchain",
    )
