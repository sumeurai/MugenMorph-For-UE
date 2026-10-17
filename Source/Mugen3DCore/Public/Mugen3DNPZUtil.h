// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mugen3DCoreTypes.h"
#include "Templates/PimplPtr.h"
#include "Mugen3DNPZUtil.generated.h"

UCLASS()
class MUGEN3DCORE_API UMugen3DNPZUtil : public UObject
{
	GENERATED_BODY()

public:
	UMugen3DNPZUtil();
	virtual ~UMugen3DNPZUtil();

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
