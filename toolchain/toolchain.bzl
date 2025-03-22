def register_all_toolchains():
    native.register_toolchains(
        "//toolchain/nucleo_l4:stm32l4xx-armv7e-mf_none_toolchain",
    )
