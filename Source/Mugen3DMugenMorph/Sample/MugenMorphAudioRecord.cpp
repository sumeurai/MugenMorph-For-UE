// Copyright 2026 Mugen3D. All Rights Reserved.

#include "MugenMorphAudioRecord.h"

#include "SumeruMugenMorphLog.h"
#include "Modules/ModuleManager.h"

FMugenMorphAudioRecord::~FMugenMorphAudioRecord()
{
	Abort();
}

bool FMugenMorphAudioRecord::Start(int32 InMaxLengthSeconds)
{
	if (bRecording)
	{
		return false;
	}

	MaxLengthSeconds = FMath::Clamp(InMaxLengthSeconds, 1, 300);

	// FAudioCapture binds the WASAPI factory in its constructor. Load the plugin first.
	if (!FModuleManager::Get().LoadModule(TEXT("AudioCapture")))
	{
		UE_LOG(LogSumeruMugenMorph, Error, TEXT("MugenMorph record: AudioCapture plugin is not available."));
		return false;
	}

	Abort();
	Capture = MakeUnique<Audio::FAudioCapture>();

	Audio::FCaptureDeviceInfo DeviceInfo;
	if (!Capture->GetCaptureDeviceInfo(DeviceInfo, INDEX_NONE))
	{
		Capture.Reset();
		UE_LOG(LogSumeruMugenMorph, Error, TEXT("MugenMorph record: no microphone device."));
		return false;
	}

	{
		FScopeLock Lock(&Mutex);
		CapturedMono.Reset();
		DeviceSampleRate = DeviceInfo.PreferredSampleRate > 0 ? DeviceInfo.PreferredSampleRate : TargetSampleRate;
	}

	Audio::FAudioCaptureDeviceParams Params;
	Params.DeviceIndex = INDEX_NONE;
	Params.SampleRate = TargetSampleRate;
	Params.NumInputChannels = 1;

	auto OpenWithParams = [this](const Audio::FAudioCaptureDeviceParams& InParams) -> bool
	{
		return Capture->OpenAudioCaptureStream(
			InParams,
			[this](const void* InAudio, int32 NumFrames, int32 NumChannels, int32 SampleRate, double StreamTime, bool bOverflow)
			{
				OnCapture(InAudio, NumFrames, NumChannels, SampleRate, StreamTime, bOverflow);
			},
			1024);
	};

	bool bOpened = OpenWithParams(Params);
	if (!bOpened)
	{
		Params.SampleRate = Audio::InvalidDeviceSampleRate;
		Params.NumInputChannels = Audio::InvalidDeviceChannelCount;
		bOpened = OpenWithParams(Params);
	}

	if (!bOpened || !Capture->StartStream())
	{
		Capture->AbortStream();
		Capture->CloseStream();
		Capture.Reset();
		UE_LOG(LogSumeruMugenMorph, Error, TEXT("MugenMorph record: failed to open capture stream."));
		return false;
	}

	bRecording = true;
	UE_LOG(LogSumeruMugenMorph, Log, TEXT("MugenMorph record: start (%s, %d Hz, %d ch)"),
		*DeviceInfo.DeviceName, DeviceInfo.PreferredSampleRate, DeviceInfo.InputChannels);
	return true;
}

bool FMugenMorphAudioRecord::Stop(TArray<uint8>& OutWavBytes)
{
	OutWavBytes.Reset();
	if (!bRecording)
	{
		return false;
	}

	if (Capture)
	{
		if (Capture->IsCapturing())
		{
			Capture->StopStream();
		}
		if (Capture->IsStreamOpen())
		{
			Capture->CloseStream();
		}
		Capture.Reset();
	}
	bRecording = false;

	TArray<float> Mono;
	int32 InRate = TargetSampleRate;
	{
		FScopeLock Lock(&Mutex);
		Mono = MoveTemp(CapturedMono);
		InRate = DeviceSampleRate > 0 ? DeviceSampleRate : TargetSampleRate;
		CapturedMono.Reset();
	}

	if (Mono.Num() <= 0)
	{
		UE_LOG(LogSumeruMugenMorph, Error, TEXT("MugenMorph record: capture was empty."));
		return false;
	}

	TArray<float> Resampled;
	ResampleMono(Mono, InRate, TargetSampleRate, Resampled);
	OutWavBytes = EncodeWavPcm16Mono(Resampled, TargetSampleRate);
	UE_LOG(LogSumeruMugenMorph, Log, TEXT("MugenMorph record: stop (%d samples @ %d Hz -> %d wav bytes)"),
		Mono.Num(), InRate, OutWavBytes.Num());
	return OutWavBytes.Num() > 44;
}

void FMugenMorphAudioRecord::Abort()
{
	if (Capture)
	{
		if (Capture->IsCapturing())
		{
			Capture->StopStream();
		}
		if (Capture->IsStreamOpen())
		{
			Capture->CloseStream();
		}
		Capture.Reset();
	}
	bRecording = false;
	FScopeLock Lock(&Mutex);
	CapturedMono.Reset();
}

void FMugenMorphAudioRecord::OnCapture(
	const void* InAudio,
	int32 NumFrames,
	int32 NumChannels,
	int32 SampleRate,
	double StreamTime,
	bool bOverflow)
{
	if (!InAudio || NumFrames <= 0 || NumChannels <= 0)
	{
		return;
	}

	FScopeLock Lock(&Mutex);
	if (SampleRate > 0)
	{
		DeviceSampleRate = SampleRate;
	}
	DownmixAndAppend(static_cast<const float*>(InAudio), NumFrames, NumChannels, CapturedMono);

	const int32 Rate = DeviceSampleRate > 0 ? DeviceSampleRate : TargetSampleRate;
	const int32 MaxSamples = Rate * MaxLengthSeconds;
	if (CapturedMono.Num() > MaxSamples)
	{
		const int32 Extra = CapturedMono.Num() - MaxSamples;
		CapturedMono.RemoveAt(0, Extra);
	}

	if (bOverflow)
	{
		UE_LOG(LogSumeruMugenMorph, Verbose, TEXT("MugenMorph record: capture overflow"));
	}
}

void FMugenMorphAudioRecord::DownmixAndAppend(const float* Interleaved, int32 NumFrames, int32 NumChannels, TArray<float>& OutMono)
{
	const int32 Start = OutMono.Num();
	OutMono.AddUninitialized(NumFrames);
	if (NumChannels == 1)
	{
		FMemory::Memcpy(OutMono.GetData() + Start, Interleaved, NumFrames * sizeof(float));
		return;
	}

	for (int32 Frame = 0; Frame < NumFrames; ++Frame)
	{
		float Acc = 0.f;
		for (int32 Channel = 0; Channel < NumChannels; ++Channel)
		{
			Acc += Interleaved[Frame * NumChannels + Channel];
		}
		OutMono[Start + Frame] = Acc / static_cast<float>(NumChannels);
	}
}

void FMugenMorphAudioRecord::ResampleMono(const TArray<float>& InSamples, int32 InRate, int32 OutRate, TArray<float>& OutSamples)
{
	if (InRate <= 0 || OutRate <= 0 || InSamples.Num() <= 0)
	{
		OutSamples.Reset();
		return;
	}
	if (InRate == OutRate)
	{
		OutSamples = InSamples;
		return;
	}

	const double Ratio = static_cast<double>(InRate) / static_cast<double>(OutRate);
	const int32 OutNum = FMath::Max(1, FMath::FloorToInt32(InSamples.Num() / Ratio));
	OutSamples.SetNumUninitialized(OutNum);
	for (int32 Index = 0; Index < OutNum; ++Index)
	{
		const double Src = static_cast<double>(Index) * Ratio;
		const int32 I0 = FMath::Clamp(FMath::FloorToInt32(Src), 0, InSamples.Num() - 1);
		const int32 I1 = FMath::Min(I0 + 1, InSamples.Num() - 1);
		const float Alpha = static_cast<float>(Src - I0);
		OutSamples[Index] = FMath::Lerp(InSamples[I0], InSamples[I1], Alpha);
	}
}

TArray<uint8> FMugenMorphAudioRecord::EncodeWavPcm16Mono(const TArray<float>& Samples, int32 SampleRate)
{
	TArray<uint8> Bytes;
	const int32 DataBytes = Samples.Num() * 2;
	Bytes.Reserve(44 + DataBytes);

	auto AppendAscii = [&Bytes](const char* Text)
	{
		Bytes.Append(reinterpret_cast<const uint8*>(Text), 4);
	};
	auto AppendU32 = [&Bytes](uint32 Value)
	{
		uint8 Chunk[4] = {
			static_cast<uint8>(Value & 0xFF),
			static_cast<uint8>((Value >> 8) & 0xFF),
			static_cast<uint8>((Value >> 16) & 0xFF),
			static_cast<uint8>((Value >> 24) & 0xFF)
		};
		Bytes.Append(Chunk, 4);
	};
	auto AppendU16 = [&Bytes](uint16 Value)
	{
		uint8 Chunk[2] = {
			static_cast<uint8>(Value & 0xFF),
			static_cast<uint8>((Value >> 8) & 0xFF)
		};
		Bytes.Append(Chunk, 2);
	};

	AppendAscii("RIFF");
	AppendU32(static_cast<uint32>(36 + DataBytes));
	AppendAscii("WAVE");
	AppendAscii("fmt ");
	AppendU32(16);
	AppendU16(1);
	AppendU16(1);
	AppendU32(static_cast<uint32>(SampleRate));
	AppendU32(static_cast<uint32>(SampleRate * 2));
	AppendU16(2);
	AppendU16(16);
	AppendAscii("data");
	AppendU32(static_cast<uint32>(DataBytes));

	Bytes.AddUninitialized(DataBytes);
	int16* Dest = reinterpret_cast<int16*>(Bytes.GetData() + 44);
	for (int32 Index = 0; Index < Samples.Num(); ++Index)
	{
		const float Clamped = FMath::Clamp(Samples[Index], -1.f, 1.f);
		Dest[Index] = static_cast<int16>(Clamped * 32767.f);
	}
	return Bytes;
}
