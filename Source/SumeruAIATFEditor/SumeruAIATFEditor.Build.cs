// Copyright SumeruAI. All Rights Reserved.

using UnrealBuildTool;

public class SumeruAIATFEditor : ModuleRules
{
	public SumeruAIATFEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"AnimGraph",
				"SumeruAIATF"
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
