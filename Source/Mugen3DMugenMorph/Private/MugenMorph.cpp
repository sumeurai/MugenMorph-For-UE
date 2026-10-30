// Copyright 2026 Mugen3D. All Rights Reserved.

#include "MugenMorph.h"
#include "MugenMorphComponent.h"
#include "MugenMorphAnim.h"
#include "Mugen3DNPZUtil.h"
#include "MugenMorphLog.h"
#include "MugenMorphSubsystem.h"
#include "Mugen3DLog.h"
#include "Mugen3DPaths.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Async/Async.h"

AMugenMorphManager* AMugenMorphManager::InstancePtr = nullptr;

AMugenMorphManager::AMugenMorphManager()
{
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
	AudioComponent->SetupAttachment(Root);
	AudioComponent->bAutoActivate = false;

	DefaultFaceAnimClass = TSoftClassPtr<UMugenMorphAnim>(
		FSoftObjectPath(TEXT("/Mugen3D/Art/Motions/NewMugenMorph_BP.NewMugenMorph_BP_C")));
}

AMugenMorphManager* AMugenMorphManager::Instance()
{
	return InstancePtr;
}

AMugenMorphManager* AMugenMorphManager::GetOrCreate(const UObject* WorldContextObject)
{
	if (UMugenMorphSubsystem* Subsystem = UMugenMorphSubsystem::Get(WorldContextObject))
	{
		return Subsystem->EnsureManager();
	}
	return InstancePtr;
}

void AMugenMorphManager::BeginPlay()
{
	Super::BeginPlay();

	if (InstancePtr == nullptr)
	{
		InstancePtr = this;
	}
	else if (InstancePtr != this)
	{
		UE_LOG(LogMugenMorph, Warning,
			TEXT("AMugenMorphManager: multiple managers in world; keeping first instance '%s', ignoring '%s'"),
			*GetNameSafe(InstancePtr),
			*GetNameSafe(this));
	}

	if (UMugenMorphSubsystem* Subsystem = UMugenMorphSubsystem::Get(this))
	{
		Subsystem->SetManager(this);
	}

	bIsPlay = true;
	bProceduralSpeechActive = false;
	bSpeechStartBroadcasted = false;
	ProceduralDrainTimer = 0.f;
	EstimatedQueuedAudioBytes = 0;
	SpeechSessionQueuedBytes = 0;
	SpeechSessionStartWorldTime = -1.f;
	Mugen3D::Mugen3DLog("AMugenMorphManager::BeginPlay");

	const FString DataDir = Mugen3D::GetMugen3DDataFolderPath();
	AudioSavePath = DataDir + TEXT("/");
	DefaultAudioPath = DataDir + TEXT("/");

	CurVHState = EVHState::Idle;
	UpdateTickEnabled();
}

void AMugenMorphManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InstancePtr == this)
	{
		InstancePtr = nullptr;
	}

	if (UMugenMorphSubsystem* Subsystem = UMugenMorphSubsystem::Get(this))
	{
		if (Subsystem->GetManager() == this)
		{
			Subsystem->SetManager(nullptr);
		}
	}

	bIsPlay = false;
	ForceStopSpeech();
	Mugen3D::Mugen3DLog("AMugenMorphManager::EndPlay");
	Super::EndPlay(EndPlayReason);
}

void AMugenMorphManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if !UE_BUILD_SHIPPING
	if (bTestOn)
	{
		bTestOn = false;
		if (NpzPaths.Num() >= 2)
		{
			PlayInternalEmote(NpzPaths[1], NpzPaths[0]);
		}
	}
#endif

	HandleMugenMorphBuffer();
	HandleProceduralAudioFinished(DeltaTime);
	HandleSubtitle(DeltaTime);
	UpdateTickEnabled();
}

void AMugenMorphManager::UpdateTickEnabled()
{
	const bool bNeedTick =
		bIsPlay
		&& (Queues.PendingAudioCount > 0
			|| Queues.PendingEmoteCount > 0
			|| PendingSubtitles.Num() > 0
			|| CurSubtitle.time > 0.f
			|| bProceduralSpeechActive
			|| FaceIsBusy());

	SetActorTickEnabled(bNeedTick || bTestOn);
}

void AMugenMorphManager::RequestTickEnabled()
{
	if (IsInGameThread())
	{
		SetActorTickEnabled(true);
		return;
	}

	TWeakObjectPtr<AMugenMorphManager> WeakThis(this);
	AsyncTask(ENamedThreads::GameThread, [WeakThis]()
	{
		if (AMugenMorphManager* Self = WeakThis.Get())
		{
			Self->SetActorTickEnabled(true);
		}
	});
}

bool AMugenMorphManager::GetMugenMorphPropertyValue(UPARAM(ref) int& FaceId, FName PropertyName, float& Value)
{
	FAudioCacheData* Data = BSCacheDataDic.Find(FaceId);
	if (!Data)
	{
		return false;
	}

	FProperty* Property = Data->StaticStruct()->FindPropertyByName(PropertyName);
	if (Property && Property->IsA<FNumericProperty>())
	{
		FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property);
		void* PropertyValuePtr = NumericProperty->ContainerPtrToValuePtr<void>(Data);
		if (NumericProperty->IsFloatingPoint())
		{
			Value = NumericProperty->GetFloatingPointPropertyValue(PropertyValuePtr);
			return true;
		}
	}
	return false;
}

void AMugenMorphManager::MugenMorphAudioFinish()
{
	NotifyAudioFinish();
}

void AMugenMorphManager::NotifyStartSpeech()
{
	OnStartSpeech.Broadcast();
	ReceiveStartSpeech();
}

void AMugenMorphManager::NotifyStopSpeech()
{
	OnStopSpeech.Broadcast();
	ReceiveStopSpeech();
}

void AMugenMorphManager::NotifyAudioStart()
{
	OnMugenMorphAudioStart.Broadcast();
	ReceiveAudioStart();
}

void AMugenMorphManager::NotifyAudioFinish()
{
	OnMugenMorphAudioFinish.Broadcast();
	ReceiveAudioFinish();
}

void AMugenMorphManager::NotifyStopMotion()
{
	OnStopMotion.Broadcast();
	ReceiveStopMotion();
}

void AMugenMorphManager::NotifyShowContent(const FString& Text)
{
	OnShowContent.Broadcast(Text);
	ReceiveShowContent(Text);
}

void AMugenMorphManager::PruneStaleFaces()
{
	TArray<int32> StaleIds;
	for (const TPair<int32, TWeakObjectPtr<UMugenMorphComponent>>& Kvp : FaceRegistry)
	{
		if (!Kvp.Value.IsValid())
		{
			StaleIds.Add(Kvp.Key);
		}
	}
	for (const int32 Id : StaleIds)
	{
		FaceRegistry.Remove(Id);
		BSCacheDataDic.Remove(Id);
	}
}

void AMugenMorphManager::ForEachLiveFace(TFunctionRef<void(int32, UMugenMorphComponent*)> Fn) const
{
	for (const TPair<int32, TWeakObjectPtr<UMugenMorphComponent>>& Kvp : FaceRegistry)
	{
		if (UMugenMorphComponent* FaceCom = Kvp.Value.Get())
		{
			Fn(Kvp.Key, FaceCom);
		}
	}
}

void AMugenMorphManager::RegisterFace(int FaceId, UMugenMorphComponent* MugenMorphComponent)
{
	if (!MugenMorphComponent)
	{
		return;
	}

	if (InstancePtr == nullptr)
	{
		InstancePtr = this;
	}

	FaceRegistry.Add(FaceId, MugenMorphComponent);
	if (!BSCacheDataDic.Contains(FaceId))
	{
		FAudioCacheData CacheData;
		CacheData.Reset();
		BSCacheDataDic.Add(FaceId, CacheData);
	}

	SetActorTickEnabled(true);
}

void AMugenMorphManager::UnRegisterFace(int FaceId)
{
	FaceRegistry.Remove(FaceId);
	BSCacheDataDic.Remove(FaceId);
}

UMugenMorphComponent* AMugenMorphManager::FindFace(int32 FaceId) const
{
	if (const TWeakObjectPtr<UMugenMorphComponent>* Found = FaceRegistry.Find(FaceId))
	{
		return Found->Get();
	}
	return nullptr;
}

bool AMugenMorphManager::HasFace(int32 FaceId) const
{
	return FindFace(FaceId) != nullptr;
}

FAudioCacheData& AMugenMorphManager::FindOrAddAudioCache(int32 FaceId)
{
	return BSCacheDataDic.FindOrAdd(FaceId);
}

bool AMugenMorphManager::TryGetAudioCache(int32 FaceId, FAudioCacheData& OutCache) const
{
	if (const FAudioCacheData* Found = BSCacheDataDic.Find(FaceId))
	{
		OutCache = *Found;
		return true;
	}
	return false;
}

bool AMugenMorphManager::TryGetCachedEmote(const FString& EmotePath, EMugenMorphDataType MugenMorphDataType, FEmoteData& OutEmote) const
{
	const FString Key = FString::Printf(TEXT("%s|%d"), *EmotePath, static_cast<int32>(MugenMorphDataType));
	if (const FEmoteData* Found = EmoteNpzCache.Find(Key))
	{
		OutEmote = *Found;
		OutEmote.aTFDataType = MugenMorphDataType;
		return true;
	}
	return false;
}

void AMugenMorphManager::CacheEmote(const FString& EmotePath, EMugenMorphDataType MugenMorphDataType, const FEmoteData& Emote)
{
	const FString Key = FString::Printf(TEXT("%s|%d"), *EmotePath, static_cast<int32>(MugenMorphDataType));
	EmoteNpzCache.Add(Key, Emote);
}

void AMugenMorphManager::PlayInternalEmote(const FString& AudioName, const FString& EmoteName)
{
	PlayInternalEmote(AudioName, EmoteName, EMugenMorphDataType::Motion, INDEX_NONE);
}

void AMugenMorphManager::PlayInternalEmote(const FString& AudioName, const FString& EmoteName, EMugenMorphDataType MugenMorphDataType)
{
	PlayInternalEmote(AudioName, EmoteName, MugenMorphDataType, INDEX_NONE);
}

void AMugenMorphManager::PlayInternalEmote(const FString& AudioName, const FString& EmoteName, EMugenMorphDataType MugenMorphDataType, int32 FaceId)
{
	if (!AudioName.IsEmpty())
	{
		PlayAudio(AudioName);
	}

	if (EmoteName.IsEmpty())
	{
		return;
	}

	const FString EmotePath = FPaths::Combine(Mugen3D::GetMugen3DDataFolderPath(), EmoteName);

	FEmoteData Cached;
	if (TryGetCachedEmote(EmotePath, MugenMorphDataType, Cached))
	{
		Cached.FaceId = FaceId;
		Cached.aTFDataType = MugenMorphDataType;
		PlayEmoteData(MoveTemp(Cached));
		RequestTickEnabled();
		return;
	}

	if (!bAsyncLoadInternalEmote || !IsInGameThread())
	{
		FAudioExpressionData Temp;
		if (UMugen3DNPZUtil::LoadCustomExpressionFromFile(EmotePath, Temp))
		{
			FEmoteData Data(Temp.Interval, MoveTemp(Temp.Expressions), MugenMorphDataType, FaceId);
			CacheEmote(EmotePath, MugenMorphDataType, Data);
			PlayEmoteData(MoveTemp(Data));
			RequestTickEnabled();
		}
		else
		{
			UE_LOG(LogMugenMorph, Warning, TEXT("PlayInternalEmote: failed to load '%s'"), *EmotePath);
		}
		return;
	}

	TWeakObjectPtr<AMugenMorphManager> WeakThis(this);
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
		[WeakThis, EmotePath, MugenMorphDataType, FaceId]()
		{
			FAudioExpressionData Temp;
			const bool bOk = UMugen3DNPZUtil::LoadCustomExpressionFromFile(EmotePath, Temp);
			AsyncTask(ENamedThreads::GameThread,
				[WeakThis, EmotePath, MugenMorphDataType, FaceId, Temp = MoveTemp(Temp), bOk]() mutable
				{
					AMugenMorphManager* Self = WeakThis.Get();
					if (!Self)
					{
						return;
					}
					if (!bOk)
					{
						UE_LOG(LogMugenMorph, Warning, TEXT("PlayInternalEmote async: failed '%s'"), *EmotePath);
						return;
					}

					FEmoteData Data(Temp.Interval, MoveTemp(Temp.Expressions), MugenMorphDataType, FaceId);
					Self->CacheEmote(EmotePath, MugenMorphDataType, Data);
					Self->PlayEmoteData(MoveTemp(Data));
					Self->RequestTickEnabled();
				});
		});
}

void AMugenMorphManager::PlayAudio(const FString& Name)
{
	AIAudioPath = FPaths::Combine(DefaultAudioPath, Name);
	PlayAudioFile(AIAudioPath);
}

void AMugenMorphManager::PlayAudioFile_Implementation(const FString& AbsoluteOrRelativePath)
{
	UE_LOG(LogMugenMorph, Log, TEXT("PlayAudioFile: %s (override in BP to play disk wav)"), *AbsoluteOrRelativePath);
}

void AMugenMorphManager::StopAudioFile_Implementation()
{
	if (AudioComponent)
	{
		AudioComponent->Stop();
	}
}

void AMugenMorphManager::TrimQueuesLocked()
{
	const int32 Cap = FMath::Max(1, MaxPendingChunks);

	while (Queues.PendingAudioCount > Cap)
	{
		TArray<uint8> Dropped;
		if (Queues.Audio.Dequeue(Dropped))
		{
			--Queues.PendingAudioCount;
			UE_LOG(LogMugenMorph, Warning, TEXT("MugenMorph audio queue backpressure: dropped oldest chunk (cap=%d)"), Cap);
		}
		else
		{
			Queues.PendingAudioCount = 0;
			break;
		}
	}

	while (Queues.PendingEmoteCount > Cap)
	{
		TSharedPtr<FEmoteData, ESPMode::ThreadSafe> Dropped;
		if (Queues.Emote.Dequeue(Dropped))
		{
			--Queues.PendingEmoteCount;
			UE_LOG(LogMugenMorph, Warning, TEXT("MugenMorph emote queue backpressure: dropped oldest chunk (cap=%d)"), Cap);
		}
		else
		{
			Queues.PendingEmoteCount = 0;
			break;
		}
	}
}

void AMugenMorphManager::DrainQueuesToLocal(
	TArray<TArray<uint8>>& OutAudioChunks,
	TArray<TSharedPtr<FEmoteData, ESPMode::ThreadSafe>>& OutEmotes)
{
	FScopeLock Lock(&QueueMutex);

	TArray<uint8> AudioChunk;
	while (Queues.Audio.Dequeue(AudioChunk))
	{
		OutAudioChunks.Add(MoveTemp(AudioChunk));
		--Queues.PendingAudioCount;
	}
	Queues.PendingAudioCount = FMath::Max(0, Queues.PendingAudioCount);

	TSharedPtr<FEmoteData, ESPMode::ThreadSafe> Emote;
	while (Queues.Emote.Dequeue(Emote))
	{
		if (Emote.IsValid())
		{
			OutEmotes.Add(MoveTemp(Emote));
		}
		--Queues.PendingEmoteCount;
	}
	Queues.PendingEmoteCount = FMath::Max(0, Queues.PendingEmoteCount);
}

void AMugenMorphManager::QueueMergedPcm(TArray<uint8>&& PcmBytes)
{
	if (!AudioComponent || PcmBytes.Num() <= 0)
	{
		return;
	}

	const int32 PlayRate = ExternalAudioSampleRate > 0 ? ExternalAudioSampleRate : 16000;
	EstimatedQueuedAudioBytes += PcmBytes.Num();

	if (!SoundStreaming || ProceduralWaveSampleRate != PlayRate)
	{
		if (AudioComponent->IsPlaying())
		{
			AudioComponent->Stop();
		}

		SoundStreaming = NewObject<USoundWaveProcedural>(this);
		SoundStreaming->bLooping = false;
		SoundStreaming->NumChannels = 1;
		SoundStreaming->bProcedural = true;
		SoundStreaming->Duration = INDEFINITELY_LOOPING_DURATION;
		SoundStreaming->SetSampleRate(PlayRate);
		ProceduralWaveSampleRate = PlayRate;
		SpeechSessionQueuedBytes = PcmBytes.Num();
		SoundStreaming->QueueAudio(PcmBytes.GetData(), PcmBytes.Num());
		AudioComponent->SetSound(SoundStreaming);
		AudioComponent->Play();
	}
	else
	{
		SpeechSessionQueuedBytes += PcmBytes.Num();
		SoundStreaming->QueueAudio(PcmBytes.GetData(), PcmBytes.Num());
		if (!AudioComponent->IsPlaying())
		{
			AudioComponent->SetSound(SoundStreaming);
			AudioComponent->Play();
		}
	}

	bProceduralSpeechActive = true;
	ProceduralDrainTimer = 0.f;
	if (!bSpeechStartBroadcasted)
	{
		if (const UWorld* World = GetWorld())
		{
			SpeechSessionStartWorldTime = World->GetTimeSeconds();
		}
		NotifyStartSpeech();
		NotifyAudioStart();
		bSpeechStartBroadcasted = true;
	}
}

void AMugenMorphManager::HandleMugenMorphBuffer()
{
	TArray<TArray<uint8>> LocalAudio;
	TArray<TSharedPtr<FEmoteData, ESPMode::ThreadSafe>> LocalEmotes;
	DrainQueuesToLocal(LocalAudio, LocalEmotes);

	if (LocalAudio.Num() > 0)
	{
		int32 TotalBytes = 0;
		for (const TArray<uint8>& Chunk : LocalAudio)
		{
			TotalBytes += Chunk.Num();
		}

		TArray<uint8> Merged;
		Merged.Reserve(TotalBytes);
		for (TArray<uint8>& Chunk : LocalAudio)
		{
			Merged.Append(MoveTemp(Chunk));
		}
		QueueMergedPcm(MoveTemp(Merged));
	}

	for (TSharedPtr<FEmoteData, ESPMode::ThreadSafe>& EmotePtr : LocalEmotes)
	{
		if (EmotePtr.IsValid())
		{
			PlayEmoteData(MoveTemp(*EmotePtr));
		}
	}
}

void AMugenMorphManager::FinishProceduralSpeech()
{
	bProceduralSpeechActive = false;
	bSpeechStartBroadcasted = false;
	ProceduralDrainTimer = 0.f;
	EstimatedQueuedAudioBytes = 0;
	SpeechSessionQueuedBytes = 0;
	SpeechSessionStartWorldTime = -1.f;
	NotifyAudioFinish();

	if (!FaceIsBusy())
	{
		NotifyStopSpeech();
	}
}

float AMugenMorphManager::GetProceduralDurationSeconds() const
{
	if (SpeechSessionQueuedBytes <= 0 || ProceduralWaveSampleRate <= 0)
	{
		return -1.f;
	}
	const int32 BytesPerSecond = ProceduralWaveSampleRate * static_cast<int32>(sizeof(int16));
	if (BytesPerSecond <= 0)
	{
		return -1.f;
	}
	return static_cast<float>(SpeechSessionQueuedBytes) / static_cast<float>(BytesPerSecond);
}

float AMugenMorphManager::GetProceduralPlaybackSeconds() const
{
	if (!bProceduralSpeechActive)
	{
		return -1.f;
	}

	float FromWall = -1.f;
	if (SpeechSessionStartWorldTime >= 0.f)
	{
		if (const UWorld* World = GetWorld())
		{
			FromWall = FMath::Max(0.f, World->GetTimeSeconds() - SpeechSessionStartWorldTime);
		}
	}

	float FromBytes = -1.f;
	if (SoundStreaming && ProceduralWaveSampleRate > 0 && SpeechSessionQueuedBytes > 0)
	{
		const int32 BytesPerSecond = ProceduralWaveSampleRate * static_cast<int32>(sizeof(int16));
		if (BytesPerSecond > 0)
		{
			const int32 RemainingBytes = FMath::Clamp(
				SoundStreaming->GetAvailableAudioByteCount(),
				0,
				SpeechSessionQueuedBytes);
			const int32 ConsumedBytes = SpeechSessionQueuedBytes - RemainingBytes;
			FromBytes = static_cast<float>(ConsumedBytes) / static_cast<float>(BytesPerSecond);
		}
	}

	float Playback = -1.f;
	if (FromWall >= 0.f && FromBytes >= 0.f)
	{
		Playback = FMath::Max(FromWall, FromBytes);
	}
	else if (FromWall >= 0.f)
	{
		Playback = FromWall;
	}
	else
	{
		Playback = FromBytes;
	}

	if (Playback < 0.f)
	{
		return -1.f;
	}

	// Do not run past known queued duration (avoids draining lips before audio ends).
	const float Duration = GetProceduralDurationSeconds();
	if (Duration > 0.f)
	{
		Playback = FMath::Min(Playback, Duration);
	}
	return Playback;
}

void AMugenMorphManager::HandleProceduralAudioFinished(float DeltaTime)
{
	if (!bProceduralSpeechActive || !SoundStreaming || !AudioComponent)
	{
		return;
	}

	// More PCM/emote still queued — not finished; avoid resetting the speech clock mid-stream.
	if (Queues.PendingAudioCount > 0 || Queues.PendingEmoteCount > 0)
	{
		ProceduralDrainTimer = 0.f;
		return;
	}

	const int32 RemainingBytes = SoundStreaming->GetAvailableAudioByteCount();
	const bool bStillPlaying = AudioComponent->IsPlaying();

	if (RemainingBytes > 0)
	{
		ProceduralDrainTimer = 0.f;
		EstimatedQueuedAudioBytes = RemainingBytes;
		return;
	}

	if (bStillPlaying)
	{
		ProceduralDrainTimer += DeltaTime;
		// Some platforms keep IsPlaying true briefly after buffer drain.
		if (ProceduralDrainTimer < ProceduralEndGraceSeconds)
		{
			return;
		}
	}
	else
	{
		ProceduralDrainTimer += DeltaTime;
		if (ProceduralDrainTimer < ProceduralEndGraceSeconds)
		{
			return;
		}
	}

	FinishProceduralSpeech();
}

void AMugenMorphManager::EnqueueExternalAudio(const TArray<uint8>& PcmBytes, int32 SampleRate)
{
	if (PcmBytes.Num() <= 0)
	{
		return;
	}

	{
		FScopeLock Lock(&QueueMutex);
		if (SampleRate > 0)
		{
			ExternalAudioSampleRate = SampleRate;
		}
		Queues.Audio.Enqueue(PcmBytes);
		++Queues.PendingAudioCount;
		TrimQueuesLocked();
	}

	RequestTickEnabled();
}

void AMugenMorphManager::EnqueueExternalEmote(const FEmoteData& EmoteData)
{
	if (EmoteData.EmoteValues.Num() <= 0)
	{
		return;
	}

	TSharedPtr<FEmoteData, ESPMode::ThreadSafe> Shared = MakeShared<FEmoteData, ESPMode::ThreadSafe>(EmoteData);
	{
		FScopeLock Lock(&QueueMutex);
		Queues.Emote.Enqueue(MoveTemp(Shared));
		++Queues.PendingEmoteCount;
		TrimQueuesLocked();
	}

	RequestTickEnabled();
}

void AMugenMorphManager::EnqueueExternalEmote(FEmoteData&& EmoteData)
{
	if (EmoteData.EmoteValues.Num() <= 0)
	{
		return;
	}

	TSharedPtr<FEmoteData, ESPMode::ThreadSafe> Shared = MakeShared<FEmoteData, ESPMode::ThreadSafe>(MoveTemp(EmoteData));
	{
		FScopeLock Lock(&QueueMutex);
		Queues.Emote.Enqueue(MoveTemp(Shared));
		++Queues.PendingEmoteCount;
		TrimQueuesLocked();
	}

	RequestTickEnabled();
}

void AMugenMorphManager::EnqueueExternalEmote(int32 FaceId, const FEmoteData& EmoteData)
{
	FEmoteData Copy = EmoteData;
	Copy.FaceId = FaceId;
	EnqueueExternalEmote(MoveTemp(Copy));
}

void AMugenMorphManager::EnqueueExternalEmote(int32 FaceId, FEmoteData&& EmoteData)
{
	EmoteData.FaceId = FaceId;
	EnqueueExternalEmote(MoveTemp(EmoteData));
}

void AMugenMorphManager::EnqueueExternalSpeech(const TArray<uint8>& PcmBytes, const FEmoteData& EmoteData, int32 SampleRate)
{
	{
		FScopeLock Lock(&QueueMutex);
		if (SampleRate > 0)
		{
			ExternalAudioSampleRate = SampleRate;
		}
		if (PcmBytes.Num() > 0)
		{
			Queues.Audio.Enqueue(PcmBytes);
			++Queues.PendingAudioCount;
		}
		if (EmoteData.EmoteValues.Num() > 0)
		{
			Queues.Emote.Enqueue(MakeShared<FEmoteData, ESPMode::ThreadSafe>(EmoteData));
			++Queues.PendingEmoteCount;
		}
		TrimQueuesLocked();
	}

	RequestTickEnabled();
}

void AMugenMorphManager::EnqueueExternalSpeech(int32 FaceId, const TArray<uint8>& PcmBytes, const FEmoteData& EmoteData, int32 SampleRate)
{
	FEmoteData Copy = EmoteData;
	Copy.FaceId = FaceId;
	EnqueueExternalSpeech(PcmBytes, Copy, SampleRate);
}

void AMugenMorphManager::EnqueueExternalSpeech(int32 FaceId, TArray<uint8>&& PcmBytes, FEmoteData&& EmoteData, int32 SampleRate)
{
	EmoteData.FaceId = FaceId;
	{
		FScopeLock Lock(&QueueMutex);
		if (SampleRate > 0)
		{
			ExternalAudioSampleRate = SampleRate;
		}
		if (PcmBytes.Num() > 0)
		{
			Queues.Audio.Enqueue(MoveTemp(PcmBytes));
			++Queues.PendingAudioCount;
		}
		if (EmoteData.EmoteValues.Num() > 0)
		{
			Queues.Emote.Enqueue(MakeShared<FEmoteData, ESPMode::ThreadSafe>(MoveTemp(EmoteData)));
			++Queues.PendingEmoteCount;
		}
		TrimQueuesLocked();
	}

	RequestTickEnabled();
}

void AMugenMorphManager::EnqueueSubtitle(const FSubtitle& Subtitle)
{
	{
		FScopeLock Lock(&QueueMutex);
		Queues.Subtitle.Enqueue(Subtitle);
	}
	RequestTickEnabled();
}

void AMugenMorphManager::ForceStopSpeech()
{
	StopAudioFile();

	{
		FScopeLock Lock(&QueueMutex);
		Queues.Emote.Empty();
		Queues.Audio.Empty();
		Queues.Subtitle.Empty();
		Queues.PendingAudioCount = 0;
		Queues.PendingEmoteCount = 0;
	}
	PendingSubtitles.Empty();
	CurSubtitle = FSubtitle();
	SubtitleTime = 0.f;

	if (AudioComponent)
	{
		AudioComponent->Stop();
	}
	SoundStreaming = nullptr;
	ProceduralWaveSampleRate = 0;
	bProceduralSpeechActive = false;
	bSpeechStartBroadcasted = false;
	ProceduralDrainTimer = 0.f;
	EstimatedQueuedAudioBytes = 0;
	SpeechSessionQueuedBytes = 0;
	SpeechSessionStartWorldTime = -1.f;

	PruneStaleFaces();
	ForEachLiveFace([](int32, UMugenMorphComponent* FaceCom)
	{
		FaceCom->Interrupt();
	});
}

void AMugenMorphManager::Interrupt()
{
	ForceStopSpeech();
}

void AMugenMorphManager::ChangeMotion(MotionType InMotionType)
{
	CurMotionType = InMotionType;
	PruneStaleFaces();
	ForEachLiveFace([InMotionType](int32, UMugenMorphComponent* FaceCom)
	{
		FaceCom->CurMotionType = InMotionType;
	});
}

bool AMugenMorphManager::FaceIsBusy() const
{
	bool bBusy = false;
	ForEachLiveFace([&bBusy](int32, UMugenMorphComponent* FaceCom)
	{
		if (FaceCom->IsBusy || FaceCom->IsPlayingEmote())
		{
			bBusy = true;
		}
	});
	return bBusy;
}

void AMugenMorphManager::PlayEmoteData(FEmoteData& EmoteData)
{
	PlayEmoteData(MoveTemp(EmoteData));
}

void AMugenMorphManager::PlayEmoteData(FEmoteData&& EmoteData)
{
	PruneStaleFaces();

	if (EmoteData.FaceId != INDEX_NONE)
	{
		if (UMugenMorphComponent* FaceCom = FindFace(EmoteData.FaceId))
		{
			FaceCom->StartEmote(MoveTemp(EmoteData));
		}
		else
		{
			UE_LOG(LogMugenMorph, Warning, TEXT("PlayEmoteData: faceId=%d not registered"), EmoteData.FaceId);
		}
		return;
	}

	// Broadcast: copy for all but last face (move into last).
	TArray<UMugenMorphComponent*> Faces;
	ForEachLiveFace([&Faces](int32, UMugenMorphComponent* FaceCom)
	{
		Faces.Add(FaceCom);
	});

	for (int32 i = 0; i < Faces.Num(); ++i)
	{
		if (i + 1 == Faces.Num())
		{
			Faces[i]->StartEmote(MoveTemp(EmoteData));
		}
		else
		{
			FEmoteData Copy = EmoteData;
			Faces[i]->StartEmote(MoveTemp(Copy));
		}
	}
}

void AMugenMorphManager::HandleSubtitle(float DeltaTime)
{
	{
		FScopeLock Lock(&QueueMutex);
		FSubtitle Sub;
		while (Queues.Subtitle.Dequeue(Sub))
		{
			PendingSubtitles.Add(MoveTemp(Sub));
		}
	}

	if (PendingSubtitles.Num() == 0 && CurSubtitle.time <= 0.f)
	{
		return;
	}

	if (CurSubtitle.time <= 0.f && PendingSubtitles.Num() > 0)
	{
		CurSubtitle = PendingSubtitles[0];
		PendingSubtitles.RemoveAt(0);
		NotifyShowContent(CurSubtitle.text);
		SubtitleTime = 0.f;
	}

	SubtitleTime += DeltaTime;
	if (CurSubtitle.time > 0.f && SubtitleTime >= CurSubtitle.time)
	{
		SubtitleTime = 0.f;
		if (PendingSubtitles.Num() > 0)
		{
			CurSubtitle = PendingSubtitles[0];
			PendingSubtitles.RemoveAt(0);
			NotifyShowContent(CurSubtitle.text);
		}
		else
		{
			NotifyShowContent(TEXT(""));
			CurSubtitle = FSubtitle();
		}
	}
}
