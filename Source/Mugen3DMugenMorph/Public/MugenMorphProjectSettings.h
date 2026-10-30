// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MugenMorphApiTypes.h"
#include "MugenMorphProjectSettings.generated.h"

/**
 * Project Settings -> Mugen3D -> MugenMorph API.
 * Lowest-priority credentials (overridden by UMugenMorphApiSettings data asset and client overrides).
 * Leave keys empty in the shipped plugin.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "MugenMorph API"))
class MUGEN3DMUGENMORPH_API UMugenMorphProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "MugenMorph|API")
	FString AccessKey;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "MugenMorph|API")
	FString SecretKey;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "MugenMorph|API")
	FString BaseUrl = TEXT("https://api.mugen3d.ai/");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "MugenMorph|API")
	FString LoginPath = TEXT("v1/access/auth");

	/** Unity production: v1/mugen-morph/offline-mesh */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "MugenMorph|API")
	FString MugenMorphMeshPath = TEXT("v1/mugen-morph/offline-mesh");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "MugenMorph|API")
	EMugenMorphApiTransport Transport = EMugenMorphApiTransport::Auto;

	virtual FName GetCategoryName() const override { return TEXT("Mugen3D"); }
	virtual FName GetSectionName() const override { return TEXT("MugenMorph API"); }
};
