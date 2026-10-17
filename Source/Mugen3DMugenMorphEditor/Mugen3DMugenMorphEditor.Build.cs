// Copyright 2026 Mugen3D. All Rights Reserved.

using UnrealBuildTool;

public class Mugen3DMugenMorphEditor : ModuleRules
{
	public Mugen3DMugenMorphEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"AnimGraph",
				"Mugen3DMugenMorph"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"LevelEditor",
				"ToolMenus",
				"Slate",
				"SlateCore",
				"InputCore",
				"Settings",
				"ContentBrowser",
				"AssetRegistry",
				"AssetTools",
				"AnimGraph",
				"AnimGraphRuntime",
				"BlueprintGraph",
				"HTTP",
				"Json",
				"Projects"
			}
		);
	}
}
