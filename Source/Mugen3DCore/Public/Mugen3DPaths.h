// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

namespace Mugen3D
{
	/** ProjectDir/Mugen3DData — creates the folder if missing. */
	inline FString GetMugen3DDataFolderPath()
	{
		const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Mugen3DData"));
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.DirectoryExists(*Path))
		{
			PlatformFile.CreateDirectoryTree(*Path);
		}
		return Path;
	}
}
