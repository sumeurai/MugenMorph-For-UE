// Copyright SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace ATFAudioDecoder
{
	/** Decode MP3 bytes to mono PCM16. Returns false for WAV/raw so the caller can keep those paths. */
	bool DecodeMp3ToMonoInt16(
		const TArray<uint8>& Compressed,
		TArray<uint8>& OutPcm16Mono,
		int32& OutSampleRate);
}
