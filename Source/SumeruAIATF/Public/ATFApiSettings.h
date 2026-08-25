// Copyright SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ATFApiTypes.h"
#include "ATFApiSettings.generated.h"

/**
 * Unity APISettingsConfig equivalent.
 * Create via Add Content -> Miscellaneous -> Data Asset -> Sumeru ATF API Settings.
 * Leave keys empty in the shipped plugin; fill from https://www.sumeruai.us/ Developers.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Sumeru ATF API Settings"))
class SUMERUAIATF_API UATFApiSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API")
	FString AccessKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API")
	FString SecretKey;

	/** e.g. https://api.sumeruai.us/ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API")
	FString BaseUrl = TEXT("https://api.sumeruai.us/");

	/** Relative to BaseUrl. Unity production: v1/access/auth */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API")
	FString LoginPath = TEXT("v1/access/auth");

	/** Relative to BaseUrl. Unity production: v1/audio-to-face/offline-mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API")
	FString ATFMeshPath = TEXT("v1/audio-to-face/offline-mesh");

	/** Auto infers OfflineProtobuf from a path containing "offline". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API")
	EATFApiTransport Transport = EATFApiTransport::Auto;

	UFUNCTION(BlueprintPure, Category = "ATF|API")
	bool HasCredentials() const
	{
		return !AccessKey.IsEmpty() && !SecretKey.IsEmpty();
	}

	UFUNCTION(BlueprintPure, Category = "ATF|API")
	FString GetLoginUrl() const
	{
		return CombineUrl(BaseUrl, LoginPath);
	}

	UFUNCTION(BlueprintPure, Category = "ATF|API")
	FString GetATFMeshUrl() const
	{
		return CombineUrl(BaseUrl, ATFMeshPath);
	}

	UFUNCTION(BlueprintPure, Category = "ATF|API")
	EATFApiTransport ResolveTransport() const
	{
		if (Transport != EATFApiTransport::Auto)
		{
			return Transport;
		}
		return ATFMeshPath.Contains(TEXT("offline"), ESearchCase::IgnoreCase)
			? EATFApiTransport::OfflineProtobuf
			: EATFApiTransport::MeshJson;
	}

	static FString CombineUrl(const FString& Url, const FString& Path)
	{
		if (Url.IsEmpty())
		{
			return Path;
		}
		if (Path.IsEmpty())
		{
			return Url;
		}
		if (Url.EndsWith(TEXT("/")) && Path.StartsWith(TEXT("/")))
		{
			return Url + Path.RightChop(1);
		}
		if (Url.EndsWith(TEXT("/")) || Path.StartsWith(TEXT("/")))
		{
			return Url + Path;
		}
		return Url + TEXT("/") + Path;
	}
};
