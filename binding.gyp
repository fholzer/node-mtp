{
    "targets": [{
        "target_name": "module",
        "cflags": ["-Wpedantic"],
        "cflags_cc": ["-Wpedantic"],
        "cflags!": ["-fno-exceptions"],
        "cflags_cc!": ["-fno-exceptions", "-Wpedantic"],
        "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CLANG_CXX_LIBRARY": "libc++",
            "MACOSX_DEPLOYMENT_TARGET": "10.7",
        },
        "msvs_settings": {
            "VCCLCompilerTool": {
                "ExceptionHandling": 1
            },
        },
        "sources": [
            "./src/module/module.cc",
            "./src/module/DeviceListEntry.cc",
            "./src/module/Device.cc"
        ],
        "libraries": [
            "-lmtp"
        ],
        "include_dirs": [
            "<!@(node -p \"require('node-addon-api').include\")"
        ]
    }],
}
