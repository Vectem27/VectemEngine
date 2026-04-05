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

-- Generate wayland files

local function script_dir()
    local info = debug.getinfo(1, "S")
    local path = info.source:sub(2) -- Remove the "@"
    return path:match("(.*/)")
end

local base = script_dir()
local folder = base .. "generated"

if not folder_exists(folder) then
    local result = os.execute("bash " .. base .. "gen-wayland.sh")
    print(result)
end