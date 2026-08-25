// Copyright SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SumeruAICoreTypes.h"
#include "ATF_Type.generated.h"

USTRUCT()
struct FSubtitle
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FString text;

	UPROPERTY(EditAnywhere)
	float time = 0.f;
};

enum EVHState
{
	Idle,
	Motion,
	Think,
	Speech,
};

UENUM()
enum class EATFDataType : uint8
{
	None,
	Motion,
	Chat,
};

UENUM(BlueprintType)
enum class MotionType : uint8
{
	Bones,
	ARKit,
	FLAME
};

USTRUCT(BlueprintType)
struct FEmoteData
{
	GENERATED_USTRUCT_BODY()

public:
	float Interval = 0.f;
	TArray<FAudioExpression> EmoteValues;
	EATFDataType aTFDataType = EATFDataType::None;

	/** INDEX_NONE = broadcast to all registered faces. */
	int32 FaceId = INDEX_NONE;

	FEmoteData() = default;

	FEmoteData(float InInterval, const TArray<FAudioExpression>& InEmoteValues, EATFDataType InATFDataType, int32 InFaceId = INDEX_NONE)
		: Interval(InInterval)
		, EmoteValues(InEmoteValues)
		, aTFDataType(InATFDataType)
		, FaceId(InFaceId)
	{
	}

	FEmoteData(float InInterval, TArray<FAudioExpression>&& InEmoteValues, EATFDataType InATFDataType, int32 InFaceId = INDEX_NONE)
		: Interval(InInterval)
		, EmoteValues(MoveTemp(InEmoteValues))
		, aTFDataType(InATFDataType)
		, FaceId(InFaceId)
	{
	}
};

/** FIFO of expression frames with compacting read index (avoids TQueue node allocs). */
struct FATFExpressionBuffer
{
	TArray<FAudioExpression> Frames;
	int32 ReadIndex = 0;

	bool IsEmpty() const { return ReadIndex >= Frames.Num(); }
	int32 Num() const { return FMath::Max(0, Frames.Num() - ReadIndex); }

	void Empty()
	{
		Frames.Reset();
		ReadIndex = 0;
	}

	void Append(const FAudioExpression& Expr)
	{
		MaybeCompact();
		Frames.Add(Expr);
	}

	void Append(FAudioExpression&& Expr)
	{
		MaybeCompact();
		Frames.Add(MoveTemp(Expr));
	}

	void AppendRange(const TArray<FAudioExpression>& InFrames)
	{
		MaybeCompact();
		Frames.Append(InFrames);
	}

	bool Dequeue(FAudioExpression& Out)
	{
		if (IsEmpty())
		{
			return false;
		}
		Out = MoveTemp(Frames[ReadIndex]);
		++ReadIndex;
		MaybeCompact();
		return true;
	}

	void MaybeCompact()
	{
		if (ReadIndex > 0 && (ReadIndex >= 64 || ReadIndex * 2 >= Frames.Num()))
		{
			Frames.RemoveAt(0, ReadIndex, false);
			ReadIndex = 0;
		}
	}
};

USTRUCT(BlueprintType)
struct FAudioCacheData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeadRoll = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeadPitch = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeadYaw = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float JawRoll = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float JawPitch = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float JawYaw = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool StartEmote = false;

	float MaleJawOpenOffset = 0.f;
	float FemaleJawOpenOffset = 0.f;

	void Reset()
	{
		HeadRoll = 0.f;
		HeadPitch = 0.f;
		HeadYaw = 0.f;
		JawRoll = 0.f;
		JawPitch = 0.f;
		JawYaw = 0.f;
		StartEmote = false;
		MaleJawOpenOffset = 0.f;
		FemaleJawOpenOffset = 0.f;
	}
};

