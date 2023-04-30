// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ZaProject : ModuleRules
{
	public ZaProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", "Niagara" });
	}
}

