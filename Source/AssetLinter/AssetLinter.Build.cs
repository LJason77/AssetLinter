// Copyright (c) 2026, LJason. All Rights Reserved.

using UnrealBuildTool;

public class AssetLinter : ModuleRules
{
    public AssetLinter(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "DeveloperSettings",
                "DataValidation",
                "BlueprintGraph",
            }
        );
    }
}