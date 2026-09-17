// Copyright 2026 SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ATFApiTypes.h"
#include "SumeruATFProjectSettings.generated.h"

/**
 * Project Settings -> SumeruAI -> ATF API.
 * Lowest-priority credentials (overridden by UATFApiSettings data asset and client overrides).
 * Leave keys empty in the shipped plugin.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "ATF API"))
class SUMERUAIATF_API USumeruATFProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "ATF|API")
	FString AccessKey;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "ATF|API")
	FString SecretKey;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "ATF|API")
	FString BaseUrl = TEXT("https://api.sumeruai.us/");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "ATF|API")
	FString LoginPath = TEXT("v1/access/auth");

	/** Unity production: v1/audio-to-face/offline-mesh */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "ATF|API")
	FString ATFMeshPath = TEXT("v1/audio-to-face/offline-mesh");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "ATF|API")
	EATFApiTransport Transport = EATFApiTransport::Auto;

	virtual FName GetCategoryName() const override { return TEXT("SumeruAI"); }
	virtual FName GetSectionName() const override { return TEXT("ATF API"); }
};
