// Copyright 2026 SumeruAI. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class SumeruAIATF : ModuleRules
{
	public SumeruAIATF(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(ModuleDirectory + "/Sample");
		PrivateIncludePaths.Add(Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "ThirdParty", "dr_mp3")));

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"SumeruAICore",
				"HTTP",
				"AnimGraphRuntime"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Json",
				"JsonUtilities",
				"DeveloperSettings",
				"Slate",
				"SlateCore",
				"InputCore",
				"AudioCaptureCore"
			}
		);

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("DesktopPlatform");
		}
	}
}
