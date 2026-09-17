// Copyright 2026 SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Unary protobuf for Unity-compatible offline ATF:
 * code=1, message=2, data=3 { fps=1, num_frames=2, audio=3, blendshapes=4, flag=5 }.
 * Hand-written decoder (no libprotobuf).
 */
struct FATFProtobufResult
{
	int32 Code = 0;
	FString Message;
	float Fps = 0.f;
	int32 NumFrames = 0;
	TArray<uint8> Audio;
	TArray<uint8> Blendshapes;

	bool HasPayload() const
	{
		return Audio.Num() > 0 || Blendshapes.Num() > 0;
	}

	bool IsSuccess() const
	{
		return (Code == 0 || Code == 200) && HasPayload();
	}
};

struct FATFProtobufParser
{
	static FATFProtobufResult Parse(const TArray<uint8>& Data);
	static FATFProtobufResult Parse(const uint8* Data, int32 Num);
};
