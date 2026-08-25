// Copyright SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

namespace SumeruAI
{
	/** ProjectDir/SumeruAIData — creates the folder if missing. */
	inline FString GetSumeruAIDataFolderPath()
	{
		const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("SumeruAIData"));
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.DirectoryExists(*Path))
		{
			PlatformFile.CreateDirectoryTree(*Path);
		}
		return Path;
	}
}
