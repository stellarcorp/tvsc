def arch_name():
    return select({
        "//platforms:arm64_linux_build": "arm64",
        "//platforms:x86_64_linux_build": "x86_64",
    })
