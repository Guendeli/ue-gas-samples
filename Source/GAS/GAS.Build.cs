// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GAS : ModuleRules
{
	public GAS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "MotionWarping","GameplayAbilities", "GameplayTags", "GameplayTasks","Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
		
		PublicIncludePaths.Add("GAS/");
	}
}
