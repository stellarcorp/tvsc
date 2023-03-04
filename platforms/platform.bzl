def arch_name():
    return select({
        "@platforms//cpu:armv7e-mf": "arm7e-mf",
        "@platforms//cpu:arm64": "arm64",
        "@platforms//cpu:x86_64": "amd64",
    })
