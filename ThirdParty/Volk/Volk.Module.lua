Volk_Rules = Module({
    PublicIncludeDirectories = {"Public"},
    PrivateIncludeDirectories = {"Private"},

    PublicDependencies = {"VulkanHeader", "VulkanLoader"},

    AdditionalMacro = { 
        { 
            Name = "_DEFAULT_SOURCE"
        }
    },
})