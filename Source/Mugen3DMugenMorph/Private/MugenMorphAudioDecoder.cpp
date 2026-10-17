// Copyright 2026 Mugen3D. All Rights Reserved.

#include "MugenMorphAudioDecoder.h"

#include "SumeruMugenMorphLog.h"

#define DR_MP3_NO_STDIO
#define DR_MP3_IMPLEMENTATION
#include "../../ThirdParty/dr_mp3/dr_mp3.h"

namespace MugenMorphAudioDecoder
{
	bool DecodeMp3ToMonoInt16(
		const TArray<uint8>& Compressed,
		TArray<uint8>& OutPcm16Mono,
		int32& OutSampleRate)
	{
		OutPcm16Mono.Reset();
		OutSampleRate = 0;
		if (Compressed.Num() < 4)
		{
			return false;
		}

		drmp3_config Config = {};
		drmp3_uint64 FrameCount = 0;
		float* Samples = drmp3_open_memory_and_read_pcm_frames_f32(
			Compressed.GetData(),
			static_cast<size_t>(Compressed.Num()),
			&Config,
			&FrameCount,
			nullptr);

		if (!Samples || FrameCount == 0 || Config.channels == 0 || Config.sampleRate == 0)
		{
			if (Samples)
			{
				drmp3_free(Samples, nullptr);
			}
			return false;
		}

		OutSampleRate = static_cast<int32>(Config.sampleRate);
		OutPcm16Mono.SetNumUninitialized(static_cast<int32>(FrameCount) * sizeof(int16));
		int16* Dst = reinterpret_cast<int16*>(OutPcm16Mono.GetData());

		const int32 Channels = static_cast<int32>(Config.channels);
		for (drmp3_uint64 Frame = 0; Frame < FrameCount; ++Frame)
		{
			float Acc = 0.0f;
			for (int32 Ch = 0; Ch < Channels; ++Ch)
			{
				Acc += Samples[Frame * Channels + Ch];
			}
			Acc /= float(Channels);
			Dst[Frame] = static_cast<int16>(FMath::Clamp(Acc, -1.0f, 1.0f) * 32767.0f);
		}

		drmp3_free(Samples, nullptr);

		UE_LOG(LogSumeruMugenMorph, Log,
			TEXT("MugenMorphAudioDecoder: MP3 decoded rate=%d ch=%d frames=%llu bytesIn=%d"),
			OutSampleRate, Channels, static_cast<unsigned long long>(FrameCount), Compressed.Num());
		return OutPcm16Mono.Num() > 0;
	}
}
