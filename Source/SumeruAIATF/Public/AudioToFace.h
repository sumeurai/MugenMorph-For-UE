// Copyright SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ATF_Type.h"
#include "Sound/SoundWaveProcedural.h"
#include "AudioToFace.generated.h"

class UATFComponent;
class UAudioComponent;
class USoundWave;
class USumeruAINPZUtil;
class UAudioToFaceAnim;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnATFAudioFinish);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnATFAudioStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnATFStartSpeech);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnATFStopSpeech);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnATFStopMotion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnATFShowContent, const FString&, Text);

UCLASS(Blueprintable, BlueprintType)
class SUMERUAIATF_API AAudioToFaceManager : public AActor
{
	GENERATED_BODY()

private:
	static AAudioToFaceManager* InstancePtr;

public:
	AAudioToFaceManager();

	/** Prefer USumeruATFSubsystem::EnsureManager() in new code. */
	static AAudioToFaceManager* Instance();

	/** Resolve via WorldSubsystem (spawns if needed). */
	static AAudioToFaceManager* GetOrCreate(const UObject* WorldContextObject);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ATF")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ATF")
	TObjectPtr<UAudioComponent> AudioComponent;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "ATF")
	bool GetATFPropertyValue(UPARAM(ref) int& FaceId, FName PropertyName, float& Value);

	UFUNCTION(BlueprintCallable, Category = "ATF")
	void ATFAudioFinish();

	UPROPERTY(BlueprintAssignable, Category = "ATF|Events")
	FOnATFAudioFinish OnATFAudioFinish;

	UPROPERTY(BlueprintAssignable, Category = "ATF|Events")
	FOnATFAudioStart OnATFAudioStart;

	UPROPERTY(BlueprintAssignable, Category = "ATF|Events")
	FOnATFStartSpeech OnStartSpeech;

	UPROPERTY(BlueprintAssignable, Category = "ATF|Events")
	FOnATFStopSpeech OnStopSpeech;

	UPROPERTY(BlueprintAssignable, Category = "ATF|Events")
	FOnATFStopMotion OnStopMotion;

	UPROPERTY(BlueprintAssignable, Category = "ATF|Events")
	FOnATFShowContent OnShowContent;

	/** Easy-to-override events inside AudioToFaceManager Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category = "ATF|Events", meta = (DisplayName = "On Start Speech"))
	void ReceiveStartSpeech();

	UFUNCTION(BlueprintImplementableEvent, Category = "ATF|Events", meta = (DisplayName = "On Stop Speech"))
	void ReceiveStopSpeech();

	UFUNCTION(BlueprintImplementableEvent, Category = "ATF|Events", meta = (DisplayName = "On Audio Start"))
	void ReceiveAudioStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "ATF|Events", meta = (DisplayName = "On Audio Finish"))
	void ReceiveAudioFinish();

	UFUNCTION(BlueprintImplementableEvent, Category = "ATF|Events", meta = (DisplayName = "On Stop Motion"))
	void ReceiveStopMotion();

	UFUNCTION(BlueprintImplementableEvent, Category = "ATF|Events", meta = (DisplayName = "On Show Content"))
	void ReceiveShowContent(const FString& Text);

	void NotifyStartSpeech();
	void NotifyStopSpeech();
	void NotifyAudioStart();
	void NotifyAudioFinish();
	void NotifyStopMotion();
	void NotifyShowContent(const FString& Text);

	void RegisterFace(int Id, UATFComponent* ATFComponent);
	void UnRegisterFace(int Id);

	UFUNCTION(BlueprintCallable, Category = "ATF")
	UATFComponent* FindFace(int32 FaceId) const;

	UFUNCTION(BlueprintCallable, Category = "ATF")
	bool HasFace(int32 FaceId) const;

	FAudioCacheData& FindOrAddAudioCache(int32 FaceId);
	bool TryGetAudioCache(int32 FaceId, FAudioCacheData& OutCache) const;

	void PlayInternalEmote(const FString& AudioName, const FString& EmoteName);
	void PlayInternalEmote(const FString& AudioName, const FString& EmoteName, EATFDataType ATFDataType);
	void PlayInternalEmote(const FString& AudioName, const FString& EmoteName, EATFDataType ATFDataType, int32 FaceId);

	void PlayAudio(const FString& Name);

	UFUNCTION(BlueprintNativeEvent, Category = "ATF")
	void PlayAudioFile(const FString& AbsoluteOrRelativePath);
	virtual void PlayAudioFile_Implementation(const FString& AbsoluteOrRelativePath);

	UFUNCTION(BlueprintNativeEvent, Category = "ATF")
	void StopAudioFile();
	virtual void StopAudioFile_Implementation();

	void HandleATFBuffer();
	void HandleSubtitle(float DeltaTime);
	void HandleProceduralAudioFinished(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "ATF")
	void ForceStopSpeech();

	UFUNCTION(BlueprintCallable, Category = "ATF", meta = (DisplayName = "Interrupt"))
	void Interrupt();

	void ChangeMotion(MotionType InMotionType);

	UFUNCTION(BlueprintCallable, Category = "ATF")
	bool FaceIsBusy() const;

	UFUNCTION(BlueprintCallable, Category = "ATF")
	void EnqueueExternalAudio(const TArray<uint8>& PcmBytes, int32 SampleRate = 16000);

	UFUNCTION(BlueprintCallable, Category = "ATF")
	void EnqueueExternalEmote(const FEmoteData& EmoteData);

	void EnqueueExternalEmote(int32 FaceId, const FEmoteData& EmoteData);
	void EnqueueExternalEmote(FEmoteData&& EmoteData);
	void EnqueueExternalEmote(int32 FaceId, FEmoteData&& EmoteData);

	UFUNCTION(BlueprintCallable, Category = "ATF")
	void EnqueueExternalSpeech(const TArray<uint8>& PcmBytes, const FEmoteData& EmoteData, int32 SampleRate = 16000);

	void EnqueueExternalSpeech(int32 FaceId, const TArray<uint8>& PcmBytes, const FEmoteData& EmoteData, int32 SampleRate = 16000);
	void EnqueueExternalSpeech(int32 FaceId, TArray<uint8>&& PcmBytes, FEmoteData&& EmoteData, int32 SampleRate = 16000);

	void EnqueueSubtitle(const FSubtitle& Subtitle);

	UFUNCTION(BlueprintCallable, Category = "ATF")
	bool IsProceduralSpeechActive() const { return bProceduralSpeechActive; }

	/**
	 * Seconds of procedural PCM already consumed / elapsed.
	 * Returns -1 if no active procedural stream / unknown.
	 */
	UFUNCTION(BlueprintCallable, Category = "ATF|Audio")
	float GetProceduralPlaybackSeconds() const;

	/** Total queued PCM duration for the current speech session (seconds). */
	UFUNCTION(BlueprintCallable, Category = "ATF|Audio")
	float GetProceduralDurationSeconds() const;

private:
	void PlayEmoteData(FEmoteData& EmoteData);
	void PlayEmoteData(FEmoteData&& EmoteData);
	void DrainQueuesToLocal(
		TArray<TArray<uint8>>& OutAudioChunks,
		TArray<TSharedPtr<FEmoteData, ESPMode::ThreadSafe>>& OutEmotes);
	void TrimQueuesLocked();
	bool TryGetCachedEmote(const FString& EmotePath, EATFDataType ATFDataType, FEmoteData& OutEmote) const;
	void CacheEmote(const FString& EmotePath, EATFDataType ATFDataType, const FEmoteData& Emote);
	void ForEachLiveFace(TFunctionRef<void(int32, UATFComponent*)> Fn) const;
	void PruneStaleFaces();
	void UpdateTickEnabled();
	void RequestTickEnabled();
	void QueueMergedPcm(TArray<uint8>&& PcmBytes);
	void FinishProceduralSpeech();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|Paths")
	FString AudioSavePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|Paths")
	FString AIAudioPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|Paths")
	FString DefaultAudioPath;

	UPROPERTY(EditAnywhere, Category = "ATF|Debug")
	bool bTestOn = false;

	UPROPERTY(EditAnywhere, Category = "ATF|Debug")
	TArray<FString> NpzPaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|Queue", meta = (ClampMin = "1"))
	int32 MaxPendingChunks = 64;

	/** Grace time after procedural buffer drains before declaring speech finished. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|Audio", meta = (ClampMin = "0.0"))
	float ProceduralEndGraceSeconds = 0.08f;

	/**
	 * Extra seconds added when estimating playback for lip-sync.
	 * Positive = mouth slightly ahead (helps when lips feel late).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|Audio", meta = (ClampMin = "-0.25", ClampMax = "0.5"))
	float LipSyncLeadSeconds = 0.02f;

	/** Load NPZ on a worker thread when cache misses. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|Emote")
	bool bAsyncLoadInternalEmote = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ATF|Anim")
	TSoftClassPtr<UAudioToFaceAnim> DefaultFaceAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF")
	FString SoundName;

	UPROPERTY(EditAnywhere, Category = "ATF")
	bool bIsPlay = true;

	EVHState CurVHState = EVHState::Idle;

	UPROPERTY(EditAnywhere, Category = "ATF")
	MotionType CurMotionType = MotionType::ARKit;

	UPROPERTY(EditAnywhere, Category = "ATF")
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

	TMap<int32, TWeakObjectPtr<UATFComponent>> FaceRegistry;
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
