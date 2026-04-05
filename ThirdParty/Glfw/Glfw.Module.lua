Glfw_Rules = Module({
    PublicIncludeDirectories = {"include"},
    PrivateIncludeDirectories = {"src", "generated"},

    AdditionalStaticLibDirectory = {},
    AdditionalStaticLib = 
    { 
        "X11", "Xrandr", "Xinerama", "Xcursor", "Xi", "GL",
        "wayland-client", "xkbcommon"
    },

    AdditionalMacro = { 
        { 
            Name = "_DEFAULT_SOURCE"
        },
        {
            Name="_GLFW_X11"
        },
        {
            Name="_GLFW_WAYLAND"
        }
    },

    CodeDir = "."
})