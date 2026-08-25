// Copyright SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SumeruAICoreTypes.generated.h"

USTRUCT(BlueprintType)
struct SUMERUAICORE_API FAudioExpression
{
	GENERATED_USTRUCT_BODY()

public:
	TArray<float> Data;
};

USTRUCT(BlueprintType)
struct SUMERUAICORE_API FAudioExpressionData
{
	GENERATED_USTRUCT_BODY()

public:
	TArray<FAudioExpression> Expressions;

	UPROPERTY(EditAnywhere)
	float Interval;

	UPROPERTY(EditAnywhere)
	int CurFrame;

	UPROPERTY(EditAnywhere)
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
