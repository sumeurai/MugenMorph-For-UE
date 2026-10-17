// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AudioCaptureCore.h"
#include "Templates/UniquePtr.h"

/**
 * Sample-only mic recorder (Unity AudioRecord).
 * Captures the default input, downmixes to mono, resamples to 16 kHz, writes 16-bit PCM WAV.
 */
class FMugenMorphAudioRecord
{
public:
	static constexpr int32 TargetSampleRate = 16000;

	FMugenMorphAudioRecord() = default;
	~FMugenMorphAudioRecord();

	FMugenMorphAudioRecord(const FMugenMorphAudioRecord&) = delete;
	FMugenMorphAudioRecord& operator=(const FMugenMorphAudioRecord&) = delete;

	bool Start(int32 MaxLengthSeconds = 300);
	bool Stop(TArray<uint8>& OutWavBytes);
	void Abort();
	bool IsRecording() const { return bRecording; }

private:
	void OnCapture(const void* InAudio, int32 NumFrames, int32 NumChannels, int32 SampleRate, double StreamTime, bool bOverflow);
	static void DownmixAndAppend(const float* Interleaved, int32 NumFrames, int32 NumChannels, TArray<float>& OutMono);
	static void ResampleMono(const TArray<float>& InSamples, int32 InRate, int32 OutRate, TArray<float>& OutSamples);
	static TArray<uint8> EncodeWavPcm16Mono(const TArray<float>& Samples, int32 SampleRate);

	TUniquePtr<Audio::FAudioCapture> Capture;
	FCriticalSection Mutex;
	TArray<float> CapturedMono;
	int32 DeviceSampleRate = TargetSampleRate;
	int32 MaxLengthSeconds = 300;
	bool bRecording = false;
};
