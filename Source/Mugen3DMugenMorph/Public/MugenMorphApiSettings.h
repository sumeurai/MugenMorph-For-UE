// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MugenMorphApiTypes.h"
#include "MugenMorphApiSettings.generated.h"

/**
 * Unity APISettingsConfig equivalent.
 * Create via Add Content -> Miscellaneous -> Data Asset -> Sumeru MugenMorph API Settings.
 * Leave keys empty in the shipped plugin; fill from https://mugen3d.ai/ Developers.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Sumeru MugenMorph API Settings"))
class MUGEN3DMUGENMORPH_API UMugenMorphApiSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API")
	FString AccessKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API")
	FString SecretKey;

	/** e.g. https://api.mugen3d.ai/ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API")
	FString BaseUrl = TEXT("https://api.mugen3d.ai/");

	/** Relative to BaseUrl. Unity production: v1/access/auth */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API")
	FString LoginPath = TEXT("v1/access/auth");

	/** Relative to BaseUrl. Unity production: v1/audio-to-face/offline-mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API")
	FString MugenMorphMeshPath = TEXT("v1/audio-to-face/offline-mesh");

	/** Auto infers OfflineProtobuf from a path containing "offline". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API")
	EMugenMorphApiTransport Transport = EMugenMorphApiTransport::Auto;

	UFUNCTION(BlueprintPure, Category = "MugenMorph|API")
	bool HasCredentials() const
	{
		return !AccessKey.IsEmpty() && !SecretKey.IsEmpty();
	}

	UFUNCTION(BlueprintPure, Category = "MugenMorph|API")
	FString GetLoginUrl() const
	{
		return CombineUrl(BaseUrl, LoginPath);
	}

	UFUNCTION(BlueprintPure, Category = "MugenMorph|API")
	FString GetMugenMorphMeshUrl() const
	{
		return CombineUrl(BaseUrl, MugenMorphMeshPath);
	}

	UFUNCTION(BlueprintPure, Category = "MugenMorph|API")
	EMugenMorphApiTransport ResolveTransport() const
	{
		if (Transport != EMugenMorphApiTransport::Auto)
		{
			return Transport;
		}
		return MugenMorphMeshPath.Contains(TEXT("offline"), ESearchCase::IgnoreCase)
			? EMugenMorphApiTransport::OfflineProtobuf
			: EMugenMorphApiTransport::MeshJson;
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
