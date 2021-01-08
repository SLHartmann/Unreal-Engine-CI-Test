using UnrealBuildTool;

public class LACRecorder : ModuleRules
{
    public LACRecorder(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new string[] {
                    "Core",
                    "InputCore",
					"CoreUObject",
                    "Engine"
                });
    }
}