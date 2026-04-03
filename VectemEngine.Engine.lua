VectemEngine_Rules = EngineRules(
{

    CompilationType = UnitCompilationTypes.Executable,

    Modules = 
    {
        /* Third Party */
        Glfw = 
        {
            Platforms = Platforms.All
        },

        /* Engine */
        Launch = 
        {
            Platforms = Platforms.All
        }
    }

})