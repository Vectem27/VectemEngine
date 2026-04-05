VectemEngine_Rules = EngineRules(
{

    CompilationType = UnitCompilationTypes.Executable,

    Modules = 
    {
        Glfw = 
        {
            Platforms = Platforms.All
        },

        Launch = 
        {
            Platforms = Platforms.All
        }
    }

})