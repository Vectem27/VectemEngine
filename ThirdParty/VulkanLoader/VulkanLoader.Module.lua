VulkanLoader_Rules = Module({
    PublicIncludeDirectories = {".", "generated"},
    PrivateIncludeDirectories = {},

    PrivateDependencies = {"VulkanHeader"},

    AdditionalStaticLibDirectory = {},
    AdditionalStaticLib = 
    { 
        "X11", "Xrandr", "Xinerama", "Xcursor", "Xi", --"GL",
        "dl", "wayland-client", "xkbcommon", "wayland-cursor", "wayland-egl", --"EGL",
        "pthread"
    },

    AdditionalMacro = { 
        { 
            Name = "_DEFAULT_SOURCE"
        },
        {
            Name = "_GNU_SOURCE"
        },
        {
            Name = "VK_DISABLE_CUDA_EXTENSIONS"
        },
        {
            Name = "VK_USE_PLATFORM_WAYLAND_KHR"
        },
        {
            Name = "VK_USE_PLATFORM_XCB_KHR"
        },
        {
            Name = "VK_USE_PLATFORM_XLIB_KHR"
        },
        {
            Name = "SYSCONFDIR",
            Value = "\"/etc\""
        },
        {
            Name = "FALLBACK_CONFIG_DIRS",
            Value = "\"/etc/xdg\""
        },
        {
            Name = "FALLBACK_DATA_DIRS",
            Value = "\"/usr/share\""
        },
        {
            Name = "VK_OBJECT_TYPE_CUDA_FUNCTION_NV",
            Value = "VK_OBJECT_TYPE_CU_FUNCTION_NVX"
        },
        {
            Name = "VK_OBJECT_TYPE_CUDA_MODULE_NV",
            Value = "VK_OBJECT_TYPE_CU_MODULE_NVX"
        }
    },

    CodeDir = "loader"
})