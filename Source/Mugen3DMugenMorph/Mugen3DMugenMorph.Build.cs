// Copyright 2026 Mugen3D. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Mugen3DMugenMorph : ModuleRules
{
	public Mugen3DMugenMorph(ReadOnlyTargetRules Target) : base(Target)
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
				"Mugen3DCore",
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
