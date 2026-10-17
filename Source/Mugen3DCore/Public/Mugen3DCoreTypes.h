// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mugen3DCoreTypes.generated.h"

USTRUCT(BlueprintType)
struct MUGEN3DCORE_API FAudioExpression
{
	GENERATED_USTRUCT_BODY()

public:
	TArray<float> Data;
};

USTRUCT(BlueprintType)
struct MUGEN3DCORE_API FAudioExpressionData
{
	GENERATED_USTRUCT_BODY()

public:
	TArray<FAudioExpression> Expressions;

	UPROPERTY(EditAnywhere, Category = "MugenMorph")
	float Interval;

	UPROPERTY(EditAnywhere, Category = "MugenMorph")
	int CurFrame;

	UPROPERTY(EditAnywhere, Category = "MugenMorph")
	float UsedTime;

public:
	void GetCurFrame(float DeltaTime)
	{
		if (Expressions.IsEmpty())
		{
			return;
		}

		if (Interval == 0)
		{
			Interval = 1.0f / 120.0f;
		}

		UsedTime += DeltaTime;
		CurFrame = FMath::FloorToInt(UsedTime / Interval);
	}

	void Reset()
	{
		Interval = 1.0f / 120.0f;
		CurFrame = 0;
		UsedTime = 0;
		Expressions.Empty();
	}
};
