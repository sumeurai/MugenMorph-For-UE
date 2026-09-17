// Copyright 2026 SumeruAI. All Rights Reserved.

using System.IO;
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

		string ThirdParty = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "ThirdParty"));
		PublicAdditionalLibraries.Add(Path.Combine(ThirdParty, "zlib", "zlib.lib"));
		PublicIncludePaths.Add(Path.Combine(ThirdParty, "zlib"));
		PrivateIncludePaths.Add(Path.Combine(ThirdParty, "cnpy"));
	}
}
