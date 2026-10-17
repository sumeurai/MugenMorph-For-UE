// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace MugenMorphAudioDecoder
{
	/** Decode MP3 bytes to mono PCM16. Returns false for WAV/raw so the caller can keep those paths. */
	bool DecodeMp3ToMonoInt16(
		const TArray<uint8>& Compressed,
		TArray<uint8>& OutPcm16Mono,
		int32& OutSampleRate);
}
