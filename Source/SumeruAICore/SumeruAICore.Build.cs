// Copyright SumeruAI. All Rights Reserved.

using UnrealBuildTool;

public class SumeruAICore : ModuleRules
{
	public SumeruAICore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			}
		);

		PublicAdditionalLibraries.Add("$(PluginDir)/ThirdParty/zlib/zlib.lib");
		PublicIncludePaths.Add("$(PluginDir)/ThirdParty/zlib");
		PrivateIncludePaths.Add(ModuleDirectory + "/Private/LoadModelData");
	}
}
