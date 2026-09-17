// Copyright 2026 SumeruAI. All Rights Reserved.

#include "AudioToFace.h"
#include "ATFComponent.h"
#include "AudioToFaceAnim.h"
#include "SumeruAINPZUtil.h"
#include "SumeruATFLog.h"
#include "SumeruATFSubsystem.h"
#include "SumeruAILog.h"
#include "SumeruAIPaths.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Async/Async.h"

AAudioToFaceManager* AAudioToFaceManager::InstancePtr = nullptr;

AAudioToFaceManager::AAudioToFaceManager()
{
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
	AudioComponent->SetupAttachment(Root);
	AudioComponent->bAutoActivate = false;

	DefaultFaceAnimClass = TSoftClassPtr<UAudioToFaceAnim>(
		FSoftObjectPath(TEXT("/SumeruAI/Art/Motions/NewATF_BP.NewATF_BP_C")));
}

AAudioToFaceManager* AAudioToFaceManager::Instance()
{
	return InstancePtr;
}

AAudioToFaceManager* AAudioToFaceManager::GetOrCreate(const UObject* WorldContextObject)
{
	if (USumeruATFSubsystem* Subsystem = USumeruATFSubsystem::Get(WorldContextObject))
	{
		return Subsystem->EnsureManager();
	}
	return InstancePtr;
}

void AAudioToFaceManager::BeginPlay()
{
	Super::BeginPlay();

	if (InstancePtr == nullptr)
	{
		InstancePtr = this;
	}
	else if (InstancePtr != this)
	{
		UE_LOG(LogSumeruATF, Warning,
			TEXT("AAudioToFaceManager: multiple managers in world; keeping first instance '%s', ignoring '%s'"),
			*GetNameSafe(InstancePtr),
			*GetNameSafe(this));
	}

	if (USumeruATFSubsystem* Subsystem = USumeruATFSubsystem::Get(this))
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
	SumeruAI::SumeruAILog("AAudioToFaceManager::BeginPlay");

	const FString DataDir = SumeruAI::GetSumeruAIDataFolderPath();
	AudioSavePath = DataDir + TEXT("/");
	DefaultAudioPath = DataDir + TEXT("/");

	CurVHState = EVHState::Idle;
	UpdateTickEnabled();
}

void AAudioToFaceManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InstancePtr == this)
	{
		InstancePtr = nullptr;
	}

	if (USumeruATFSubsystem* Subsystem = USumeruATFSubsystem::Get(this))
	{
		if (Subsystem->GetManager() == this)
		{
			Subsystem->SetManager(nullptr);
		}
	}

	bIsPlay = false;
	ForceStopSpeech();
	SumeruAI::SumeruAILog("AAudioToFaceManager::EndPlay");
	Super::EndPlay(EndPlayReason);
}

void AAudioToFaceManager::Tick(float DeltaTime)
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

	HandleATFBuffer();
	HandleProceduralAudioFinished(DeltaTime);
	HandleSubtitle(DeltaTime);
	UpdateTickEnabled();
}

void AAudioToFaceManager::UpdateTickEnabled()
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

void AAudioToFaceManager::RequestTickEnabled()
{
	if (IsInGameThread())
	{
		SetActorTickEnabled(true);
		return;
	}

	TWeakObjectPtr<AAudioToFaceManager> WeakThis(this);
	AsyncTask(ENamedThreads::GameThread, [WeakThis]()
	{
		if (AAudioToFaceManager* Self = WeakThis.Get())
		{
			Self->SetActorTickEnabled(true);
		}
	});
}

bool AAudioToFaceManager::GetATFPropertyValue(UPARAM(ref) int& FaceId, FName PropertyName, float& Value)
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

void AAudioToFaceManager::ATFAudioFinish()
{
	NotifyAudioFinish();
}

void AAudioToFaceManager::NotifyStartSpeech()
{
	OnStartSpeech.Broadcast();
	ReceiveStartSpeech();
}

void AAudioToFaceManager::NotifyStopSpeech()
{
	OnStopSpeech.Broadcast();
	ReceiveStopSpeech();
}

void AAudioToFaceManager::NotifyAudioStart()
{
	OnATFAudioStart.Broadcast();
	ReceiveAudioStart();
}

void AAudioToFaceManager::NotifyAudioFinish()
{
	OnATFAudioFinish.Broadcast();
	ReceiveAudioFinish();
}

void AAudioToFaceManager::NotifyStopMotion()
{
	OnStopMotion.Broadcast();
	ReceiveStopMotion();
}

void AAudioToFaceManager::NotifyShowContent(const FString& Text)
{
	OnShowContent.Broadcast(Text);
	ReceiveShowContent(Text);
}

void AAudioToFaceManager::PruneStaleFaces()
{
	TArray<int32> StaleIds;
	for (const TPair<int32, TWeakObjectPtr<UATFComponent>>& Kvp : FaceRegistry)
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

void AAudioToFaceManager::ForEachLiveFace(TFunctionRef<void(int32, UATFComponent*)> Fn) const
{
	for (const TPair<int32, TWeakObjectPtr<UATFComponent>>& Kvp : FaceRegistry)
	{
		if (UATFComponent* FaceCom = Kvp.Value.Get())
		{
			Fn(Kvp.Key, FaceCom);
		}
	}
}

void AAudioToFaceManager::RegisterFace(int FaceId, UATFComponent* ATFComponent)
{
	if (!ATFComponent)
	{
		return;
	}

	if (InstancePtr == nullptr)
	{
		InstancePtr = this;
	}

	FaceRegistry.Add(FaceId, ATFComponent);
	if (!BSCacheDataDic.Contains(FaceId))
	{
		FAudioCacheData CacheData;
		CacheData.Reset();
		BSCacheDataDic.Add(FaceId, CacheData);
	}

	SetActorTickEnabled(true);
}

void AAudioToFaceManager::UnRegisterFace(int FaceId)
{
	FaceRegistry.Remove(FaceId);
	BSCacheDataDic.Remove(FaceId);
}

UATFComponent* AAudioToFaceManager::FindFace(int32 FaceId) const
{
	if (const TWeakObjectPtr<UATFComponent>* Found = FaceRegistry.Find(FaceId))
	{
		return Found->Get();
	}
	return nullptr;
}

bool AAudioToFaceManager::HasFace(int32 FaceId) const
{
	return FindFace(FaceId) != nullptr;
}

FAudioCacheData& AAudioToFaceManager::FindOrAddAudioCache(int32 FaceId)
{
	return BSCacheDataDic.FindOrAdd(FaceId);
}

bool AAudioToFaceManager::TryGetAudioCache(int32 FaceId, FAudioCacheData& OutCache) const
{
	if (const FAudioCacheData* Found = BSCacheDataDic.Find(FaceId))
	{
		OutCache = *Found;
		return true;
	}
	return false;
}

bool AAudioToFaceManager::TryGetCachedEmote(const FString& EmotePath, EATFDataType ATFDataType, FEmoteData& OutEmote) const
{
	const FString Key = FString::Printf(TEXT("%s|%d"), *EmotePath, static_cast<int32>(ATFDataType));
	if (const FEmoteData* Found = EmoteNpzCache.Find(Key))
	{
		OutEmote = *Found;
		OutEmote.aTFDataType = ATFDataType;
		return true;
	}
	return false;
}

void AAudioToFaceManager::CacheEmote(const FString& EmotePath, EATFDataType ATFDataType, const FEmoteData& Emote)
{
	const FString Key = FString::Printf(TEXT("%s|%d"), *EmotePath, static_cast<int32>(ATFDataType));
	EmoteNpzCache.Add(Key, Emote);
}

void AAudioToFaceManager::PlayInternalEmote(const FString& AudioName, const FString& EmoteName)
{
	PlayInternalEmote(AudioName, EmoteName, EATFDataType::Motion, INDEX_NONE);
}

void AAudioToFaceManager::PlayInternalEmote(const FString& AudioName, const FString& EmoteName, EATFDataType ATFDataType)
{
	PlayInternalEmote(AudioName, EmoteName, ATFDataType, INDEX_NONE);
}

void AAudioToFaceManager::PlayInternalEmote(const FString& AudioName, const FString& EmoteName, EATFDataType ATFDataType, int32 FaceId)
{
	if (!AudioName.IsEmpty())
	{
		PlayAudio(AudioName);
	}

	if (EmoteName.IsEmpty())
	{
		return;
	}

	const FString EmotePath = FPaths::Combine(SumeruAI::GetSumeruAIDataFolderPath(), EmoteName);

	FEmoteData Cached;
	if (TryGetCachedEmote(EmotePath, ATFDataType, Cached))
	{
		Cached.FaceId = FaceId;
		Cached.aTFDataType = ATFDataType;
		PlayEmoteData(MoveTemp(Cached));
		RequestTickEnabled();
		return;
	}

	if (!bAsyncLoadInternalEmote || !IsInGameThread())
	{
		FAudioExpressionData Temp;
		if (USumeruAINPZUtil::LoadCustomExpressionFromFile(EmotePath, Temp))
		{
			FEmoteData Data(Temp.Interval, MoveTemp(Temp.Expressions), ATFDataType, FaceId);
			CacheEmote(EmotePath, ATFDataType, Data);
			PlayEmoteData(MoveTemp(Data));
			RequestTickEnabled();
		}
		else
		{
			UE_LOG(LogSumeruATF, Warning, TEXT("PlayInternalEmote: failed to load '%s'"), *EmotePath);
		}
		return;
	}

	TWeakObjectPtr<AAudioToFaceManager> WeakThis(this);
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
		[WeakThis, EmotePath, ATFDataType, FaceId]()
		{
			FAudioExpressionData Temp;
			const bool bOk = USumeruAINPZUtil::LoadCustomExpressionFromFile(EmotePath, Temp);
			AsyncTask(ENamedThreads::GameThread,
				[WeakThis, EmotePath, ATFDataType, FaceId, Temp = MoveTemp(Temp), bOk]() mutable
				{
					AAudioToFaceManager* Self = WeakThis.Get();
					if (!Self)
					{
						return;
					}
					if (!bOk)
					{
						UE_LOG(LogSumeruATF, Warning, TEXT("PlayInternalEmote async: failed '%s'"), *EmotePath);
						return;
					}

					FEmoteData Data(Temp.Interval, MoveTemp(Temp.Expressions), ATFDataType, FaceId);
					Self->CacheEmote(EmotePath, ATFDataType, Data);
					Self->PlayEmoteData(MoveTemp(Data));
					Self->RequestTickEnabled();
				});
		});
}

void AAudioToFaceManager::PlayAudio(const FString& Name)
{
	AIAudioPath = FPaths::Combine(DefaultAudioPath, Name);
	PlayAudioFile(AIAudioPath);
}

void AAudioToFaceManager::PlayAudioFile_Implementation(const FString& AbsoluteOrRelativePath)
{
	UE_LOG(LogSumeruATF, Log, TEXT("PlayAudioFile: %s (override in BP to play disk wav)"), *AbsoluteOrRelativePath);
}

void AAudioToFaceManager::StopAudioFile_Implementation()
{
	if (AudioComponent)
	{
		AudioComponent->Stop();
	}
}

void AAudioToFaceManager::TrimQueuesLocked()
{
	const int32 Cap = FMath::Max(1, MaxPendingChunks);

	while (Queues.PendingAudioCount > Cap)
	{
		TArray<uint8> Dropped;
		if (Queues.Audio.Dequeue(Dropped))
		{
			--Queues.PendingAudioCount;
			UE_LOG(LogSumeruATF, Warning, TEXT("ATF audio queue backpressure: dropped oldest chunk (cap=%d)"), Cap);
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
			UE_LOG(LogSumeruATF, Warning, TEXT("ATF emote queue backpressure: dropped oldest chunk (cap=%d)"), Cap);
		}
		else
		{
			Queues.PendingEmoteCount = 0;
			break;
		}
	}
}

void AAudioToFaceManager::DrainQueuesToLocal(
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

void AAudioToFaceManager::QueueMergedPcm(TArray<uint8>&& PcmBytes)
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

void AAudioToFaceManager::HandleATFBuffer()
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

void AAudioToFaceManager::FinishProceduralSpeech()
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

float AAudioToFaceManager::GetProceduralDurationSeconds() const
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

float AAudioToFaceManager::GetProceduralPlaybackSeconds() const
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

void AAudioToFaceManager::HandleProceduralAudioFinished(float DeltaTime)
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

void AAudioToFaceManager::EnqueueExternalAudio(const TArray<uint8>& PcmBytes, int32 SampleRate)
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

void AAudioToFaceManager::EnqueueExternalEmote(const FEmoteData& EmoteData)
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

void AAudioToFaceManager::EnqueueExternalEmote(FEmoteData&& EmoteData)
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

void AAudioToFaceManager::EnqueueExternalEmote(int32 FaceId, const FEmoteData& EmoteData)
{
	FEmoteData Copy = EmoteData;
	Copy.FaceId = FaceId;
	EnqueueExternalEmote(MoveTemp(Copy));
}

void AAudioToFaceManager::EnqueueExternalEmote(int32 FaceId, FEmoteData&& EmoteData)
{
	EmoteData.FaceId = FaceId;
	EnqueueExternalEmote(MoveTemp(EmoteData));
}

void AAudioToFaceManager::EnqueueExternalSpeech(const TArray<uint8>& PcmBytes, const FEmoteData& EmoteData, int32 SampleRate)
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

void AAudioToFaceManager::EnqueueExternalSpeech(int32 FaceId, const TArray<uint8>& PcmBytes, const FEmoteData& EmoteData, int32 SampleRate)
{
	FEmoteData Copy = EmoteData;
	Copy.FaceId = FaceId;
	EnqueueExternalSpeech(PcmBytes, Copy, SampleRate);
}

void AAudioToFaceManager::EnqueueExternalSpeech(int32 FaceId, TArray<uint8>&& PcmBytes, FEmoteData&& EmoteData, int32 SampleRate)
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

void AAudioToFaceManager::EnqueueSubtitle(const FSubtitle& Subtitle)
{
	{
		FScopeLock Lock(&QueueMutex);
		Queues.Subtitle.Enqueue(Subtitle);
	}
	RequestTickEnabled();
}

void AAudioToFaceManager::ForceStopSpeech()
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
	ForEachLiveFace([](int32, UATFComponent* FaceCom)
	{
		FaceCom->Interrupt();
	});
}

void AAudioToFaceManager::Interrupt()
{
	ForceStopSpeech();
}

void AAudioToFaceManager::ChangeMotion(MotionType InMotionType)
{
	CurMotionType = InMotionType;
	PruneStaleFaces();
	ForEachLiveFace([InMotionType](int32, UATFComponent* FaceCom)
	{
		FaceCom->CurMotionType = InMotionType;
	});
}

bool AAudioToFaceManager::FaceIsBusy() const
{
	bool bBusy = false;
	ForEachLiveFace([&bBusy](int32, UATFComponent* FaceCom)
	{
		if (FaceCom->IsBusy || FaceCom->IsPlayingEmote())
		{
			bBusy = true;
		}
	});
	return bBusy;
}

void AAudioToFaceManager::PlayEmoteData(FEmoteData& EmoteData)
{
	PlayEmoteData(MoveTemp(EmoteData));
}

void AAudioToFaceManager::PlayEmoteData(FEmoteData&& EmoteData)
{
	PruneStaleFaces();

	if (EmoteData.FaceId != INDEX_NONE)
	{
		if (UATFComponent* FaceCom = FindFace(EmoteData.FaceId))
		{
			FaceCom->StartEmote(MoveTemp(EmoteData));
		}
		else
		{
			UE_LOG(LogSumeruATF, Warning, TEXT("PlayEmoteData: faceId=%d not registered"), EmoteData.FaceId);
		}
		return;
	}

	// Broadcast: copy for all but last face (move into last).
	TArray<UATFComponent*> Faces;
	ForEachLiveFace([&Faces](int32, UATFComponent* FaceCom)
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

void AAudioToFaceManager::HandleSubtitle(float DeltaTime)
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
