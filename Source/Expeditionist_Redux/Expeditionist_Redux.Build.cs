// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Expeditionist_Redux : ModuleRules
{
	public Expeditionist_Redux(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
