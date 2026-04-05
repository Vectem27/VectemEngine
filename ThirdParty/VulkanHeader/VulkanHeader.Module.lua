VulkanHeader_Rules = Module({
    PublicIncludeDirectories = {"include"},
    PrivateIncludeDirectories = {},

    AdditionalStaticLibDirectory = {},
    AdditionalStaticLib = 
    { 
        --"X11", "Xrandr", "Xinerama", "Xcursor", "Xi", "GL",
        --"wayland-client", "xkbcommon"
    },

    AdditionalMacro = { 
        { 
            Name = "_DEFAULT_SOURCE"
        }
    },

    CodeDir = "."
})