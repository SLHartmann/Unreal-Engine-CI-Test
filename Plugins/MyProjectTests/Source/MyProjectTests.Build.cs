using UnrealBuildTool;

public class MyProjectTests : ModuleRules
{
    public MyProjectTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new string[] {
                    "Core",
                    "Engine",
                    "CoreUObject",
                    "MyProject",
                    "UnrealED",
					"InputCore",
                    "SlateCore",
                    "Slate",
                    "MyProject",
                    "AutomationDriver"
                });
    }
}