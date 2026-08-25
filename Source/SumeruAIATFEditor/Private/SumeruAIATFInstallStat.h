// Copyright SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HttpFwd.h"

class FSumeruAIATFInstallStat
{
public:
	static void Startup();
	static void Shutdown();

private:
	static void ScheduleTryReport();
	static void TryReport();
	static void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	static FString BuildRequestJson();
	static FString GetOrCreateAnonId();
	static FString GetSdkVersion();
	static bool IsOptionalBlank(
		const FString& Product,
		const FString& PackageName,
		const FString& AnonId,
		const FString& SdkVersion,
		const FString& EngineVersion,
		const FString& MachineId,
		const FString& KeyId);
	static void LogIfDebug(const FString& Message);

	static bool bStarted;
	static FDelegateHandle PostEngineInitHandle;
};
