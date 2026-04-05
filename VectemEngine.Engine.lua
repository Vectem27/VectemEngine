VectemEngine_Rules = EngineRules(
{

    CompilationType = UnitCompilationTypes.Executable,

    Modules = 
    {
        -- Third party modules
        VulkanHeader =
        {
            Platforms = Platforms.All
        },
        VulkanLoader = 
        {
            Platforms = Platforms.All
        },
        Glfw = 
        {
            Platforms = Platforms.All
        },

        -- Engine modules

        Launch = 
        {
            Platforms = Platforms.All
        }
    }

})