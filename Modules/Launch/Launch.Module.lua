Launch_Rules = Module({
    PrivateDependencies = {"Glfw", "VulkanHeader", "VulkanLoader", "Volk"},

    AdditionalMacro = { 
        { 
            Name = "FORCE_GLFW_X11"
        }
    }
})