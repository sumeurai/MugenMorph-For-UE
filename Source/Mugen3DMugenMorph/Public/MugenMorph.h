// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MugenMorph_Type.h"
#include "Sound/SoundWaveProcedural.h"
#include "MugenMorph.generated.h"

class UMugenMorphComponent;
class UAudioComponent;
class USoundWave;
class UMugen3DNPZUtil;
class UMugenMorphAnim;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMugenMorphAudioFinish);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMugenMorphAudioStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMugenMorphStartSpeech);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMugenMorphStopSpeech);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMugenMorphStopMotion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMugenMorphShowContent, const FString&, Text);

UCLASS(Blueprintable, BlueprintType)
class MUGEN3DMUGENMORPH_API AMugenMorphManager : public AActor
{
	GENERATED_BODY()

private:
	static AMugenMorphManager* InstancePtr;

public:
	AMugenMorphManager();

	/** Prefer UMugenMorphSubsystem::EnsureManager() in new code. */
	static AMugenMorphManager* Instance();

	/** Resolve via WorldSubsystem (spawns if needed). */
	static AMugenMorphManager* GetOrCreate(const UObject* WorldContextObject);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MugenMorph")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MugenMorph")
	TObjectPtr<UAudioComponent> AudioComponent;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "MugenMorph")
	bool GetMugenMorphPropertyValue(UPARAM(ref) int& FaceId, FName PropertyName, float& Value);

	UFUNCTION(BlueprintCallable, Category = "MugenMorph")
	void MugenMorphAudioFinish();

	UPROPERTY(BlueprintAssignable, Category = "MugenMorph|Events")
	FOnMugenMorphAudioFinish OnMugenMorphAudioFinish;

	UPROPERTY(BlueprintAssignable, Category = "MugenMorph|Events")
	FOnMugenMorphAudioStart OnMugenMorphAudioStart;

	UPROPERTY(BlueprintAssignable, Category = "MugenMorph|Events")
	FOnMugenMorphStartSpeech OnStartSpeech;

	UPROPERTY(BlueprintAssignable, Category = "MugenMorph|Events")
	FOnMugenMorphStopSpeech OnStopSpeech;

	UPROPERTY(BlueprintAssignable, Category = "MugenMorph|Events")
	FOnMugenMorphStopMotion OnStopMotion;

	UPROPERTY(BlueprintAssignable, Category = "MugenMorph|Events")
	FOnMugenMorphShowContent OnShowContent;

	/** Easy-to-override events inside MugenMorphManager Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category = "MugenMorph|Events", meta = (DisplayName = "On Start Speech"))
	void ReceiveStartSpeech();

	UFUNCTION(BlueprintImplementableEvent, Category = "MugenMorph|Events", meta = (DisplayName = "On Stop Speech"))
	void ReceiveStopSpeech();

	UFUNCTION(BlueprintImplementableEvent, Category = "MugenMorph|Events", meta = (DisplayName = "On Audio Start"))
	void ReceiveAudioStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "MugenMorph|Events", meta = (DisplayName = "On Audio Finish"))
	void ReceiveAudioFinish();

	UFUNCTION(BlueprintImplementableEvent, Category = "MugenMorph|Events", meta = (DisplayName = "On Stop Motion"))
	void ReceiveStopMotion();

	UFUNCTION(BlueprintImplementableEvent, Category = "MugenMorph|Events", meta = (DisplayName = "On Show Content"))
	void ReceiveShowContent(const FString& Text);

	void NotifyStartSpeech();
	void NotifyStopSpeech();
	void NotifyAudioStart();
	void NotifyAudioFinish();
	void NotifyStopMotion();
	void NotifyShowContent(const FString& Text);

	void RegisterFace(int Id, UMugenMorphComponent* MugenMorphComponent);
	void UnRegisterFace(int Id);

	UFUNCTION(BlueprintCallable, Category = "MugenMorph")
	UMugenMorphComponent* FindFace(int32 FaceId) const;

	UFUNCTION(BlueprintCallable, Category = "MugenMorph")
	bool HasFace(int32 FaceId) const;

	FAudioCacheData& FindOrAddAudioCache(int32 FaceId);
	bool TryGetAudioCache(int32 FaceId, FAudioCacheData& OutCache) const;

	void PlayInternalEmote(const FString& AudioName, const FString& EmoteName);
	void PlayInternalEmote(const FString& AudioName, const FString& EmoteName, EMugenMorphDataType MugenMorphDataType);
	void PlayInternalEmote(const FString& AudioName, const FString& EmoteName, EMugenMorphDataType MugenMorphDataType, int32 FaceId);

	void PlayAudio(const FString& Name);

	UFUNCTION(BlueprintNativeEvent, Category = "MugenMorph")
	void PlayAudioFile(const FString& AbsoluteOrRelativePath);
	virtual void PlayAudioFile_Implementation(const FString& AbsoluteOrRelativePath);

	UFUNCTION(BlueprintNativeEvent, Category = "MugenMorph")
	void StopAudioFile();
	virtual void StopAudioFile_Implementation();

	void HandleMugenMorphBuffer();
	void HandleSubtitle(float DeltaTime);
	void HandleProceduralAudioFinished(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "MugenMorph")
	void ForceStopSpeech();

	UFUNCTION(BlueprintCallable, Category = "MugenMorph", meta = (DisplayName = "Interrupt"))
	void Interrupt();

	void ChangeMotion(MotionType InMotionType);

	UFUNCTION(BlueprintCallable, Category = "MugenMorph")
	bool FaceIsBusy() const;

	UFUNCTION(BlueprintCallable, Category = "MugenMorph")
	void EnqueueExternalAudio(const TArray<uint8>& PcmBytes, int32 SampleRate = 16000);

	UFUNCTION(BlueprintCallable, Category = "MugenMorph")
	void EnqueueExternalEmote(const FEmoteData& EmoteData);

	void EnqueueExternalEmote(int32 FaceId, const FEmoteData& EmoteData);
	void EnqueueExternalEmote(FEmoteData&& EmoteData);
	void EnqueueExternalEmote(int32 FaceId, FEmoteData&& EmoteData);

	UFUNCTION(BlueprintCallable, Category = "MugenMorph")
	void EnqueueExternalSpeech(const TArray<uint8>& PcmBytes, const FEmoteData& EmoteData, int32 SampleRate = 16000);

	void EnqueueExternalSpeech(int32 FaceId, const TArray<uint8>& PcmBytes, const FEmoteData& EmoteData, int32 SampleRate = 16000);
	void EnqueueExternalSpeech(int32 FaceId, TArray<uint8>&& PcmBytes, FEmoteData&& EmoteData, int32 SampleRate = 16000);

	void EnqueueSubtitle(const FSubtitle& Subtitle);

	UFUNCTION(BlueprintCallable, Category = "MugenMorph")
	bool IsProceduralSpeechActive() const { return bProceduralSpeechActive; }

	/**
	 * Seconds of procedural PCM already consumed / elapsed.
	 * Returns -1 if no active procedural stream / unknown.
	 */
	UFUNCTION(BlueprintCallable, Category = "MugenMorph|Audio")
	float GetProceduralPlaybackSeconds() const;

	/** Total queued PCM duration for the current speech session (seconds). */
	UFUNCTION(BlueprintCallable, Category = "MugenMorph|Audio")
	float GetProceduralDurationSeconds() const;

private:
	void PlayEmoteData(FEmoteData& EmoteData);
	void PlayEmoteData(FEmoteData&& EmoteData);
	void DrainQueuesToLocal(
		TArray<TArray<uint8>>& OutAudioChunks,
		TArray<TSharedPtr<FEmoteData, ESPMode::ThreadSafe>>& OutEmotes);
	void TrimQueuesLocked();
	bool TryGetCachedEmote(const FString& EmotePath, EMugenMorphDataType MugenMorphDataType, FEmoteData& OutEmote) const;
	void CacheEmote(const FString& EmotePath, EMugenMorphDataType MugenMorphDataType, const FEmoteData& Emote);
	void ForEachLiveFace(TFunctionRef<void(int32, UMugenMorphComponent*)> Fn) const;
	void PruneStaleFaces();
	void UpdateTickEnabled();
	void RequestTickEnabled();
	void QueueMergedPcm(TArray<uint8>&& PcmBytes);
	void FinishProceduralSpeech();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|Paths")
	FString AudioSavePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|Paths")
	FString AIAudioPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|Paths")
	FString DefaultAudioPath;

	UPROPERTY(EditAnywhere, Category = "MugenMorph|Debug")
	bool bTestOn = false;

	UPROPERTY(EditAnywhere, Category = "MugenMorph|Debug")
	TArray<FString> NpzPaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|Queue", meta = (ClampMin = "1"))
	int32 MaxPendingChunks = 64;

	/** Grace time after procedural buffer drains before declaring speech finished. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|Audio", meta = (ClampMin = "0.0"))
	float ProceduralEndGraceSeconds = 0.08f;

	/**
	 * Extra seconds added when estimating playback for lip-sync.
	 * Positive = mouth slightly ahead (helps when lips feel late).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|Audio", meta = (ClampMin = "-0.25", ClampMax = "0.5"))
	float LipSyncLeadSeconds = 0.02f;

	/** Load NPZ on a worker thread when cache misses. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|Emote")
	bool bAsyncLoadInternalEmote = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MugenMorph|Anim")
	TSoftClassPtr<UMugenMorphAnim> DefaultFaceAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph")
	FString SoundName;

	UPROPERTY(EditAnywhere, Category = "MugenMorph")
	bool bIsPlay = true;

	EVHState CurVHState = EVHState::Idle;

	UPROPERTY(EditAnywhere, Category = "MugenMorph")
	MotionType CurMotionType = MotionType::ARKit;

	UPROPERTY(EditAnywhere, Category = "MugenMorph")
	int32 Sex = 0;

	int32 ExternalAudioSampleRate = 16000;

protected:
	UPROPERTY(Transient)
	TObjectPtr<USoundWaveProcedural> SoundStreaming;

	int32 ProceduralWaveSampleRate = 0;

	FSubtitle CurSubtitle;
	float SubtitleTime = 0.f;
	TArray<FSubtitle> PendingSubtitles;

private:
	struct FSpeechQueues
	{
		TQueue<TArray<uint8>> Audio;
		TQueue<TSharedPtr<FEmoteData, ESPMode::ThreadSafe>> Emote;
		TQueue<FSubtitle> Subtitle;
		int32 PendingAudioCount = 0;
		int32 PendingEmoteCount = 0;
	};

	mutable FCriticalSection QueueMutex;
	FSpeechQueues Queues;

	TMap<int32, TWeakObjectPtr<UMugenMorphComponent>> FaceRegistry;
	TMap<int32, FAudioCacheData> BSCacheDataDic;
	TMap<FString, FEmoteData> EmoteNpzCache;

	bool bProceduralSpeechActive = false;
	bool bSpeechStartBroadcasted = false;
	float ProceduralDrainTimer = 0.f;
	int32 EstimatedQueuedAudioBytes = 0;
	/** Total PCM bytes queued for the current procedural speech session. */
	int32 SpeechSessionQueuedBytes = 0;
	/** World time when procedural Play() started; <0 if inactive. */
	float SpeechSessionStartWorldTime = -1.f;
};
