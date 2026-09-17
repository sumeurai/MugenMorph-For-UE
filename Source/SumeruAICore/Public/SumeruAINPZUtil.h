// Copyright 2026 SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SumeruAICoreTypes.h"
#include "Templates/PimplPtr.h"
#include "SumeruAINPZUtil.generated.h"

UCLASS()
class SUMERUAICORE_API USumeruAINPZUtil : public UObject
{
	GENERATED_BODY()

public:
	USumeruAINPZUtil();
	virtual ~USumeruAINPZUtil();

	void LoadNPZ(const FString& FilePath);
	TArray<float> GetVertices();
	TArray<uint8> GetTextureData();

	FVector GetBoneData(const FString& Name);
	FVector GetColor();
	FVector GetCheekColor();
	FVector GetColorShift();

	FVector GetHeadTranslate();
	FVector GetHeadScale();
	TArray<float> GetMale();
	TArray<float> GetMaleVertices();
	FVector GetMaleScale();
	float GetMaleScaleF();

	TArray<float> GetFemale();
	TArray<float> GetFemaleVertices();
	FVector GetFemaleScale();
	float GetFemaleScaleF();

	int Width;
	int Height;

	FAudioExpressionData GetCustomExpressionData();

	/** Thread-safe NPZ → expression load (no UObject required). Safe for worker threads. */
	static bool LoadCustomExpressionFromFile(const FString& FilePath, FAudioExpressionData& OutData);

private:
	struct FNpzState;
	TPimplPtr<FNpzState> State;
};
