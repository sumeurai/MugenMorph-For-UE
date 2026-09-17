// Copyright 2026 SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ATF_Type.h"
#include "AudioToFaceAnim.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "HAL/CriticalSection.h"
#include "ATFComponent.generated.h"

class AAudioToFaceManager;
class AActor;
class USkeletalMesh;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SUMERUAIATF_API UATFComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UATFComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void StartEmote(FEmoteData& EmoteData);
	void StartEmote(FEmoteData&& EmoteData);
	void PlayEmote(float DeltaTime);

	void SetBSData(FEmoteData&& EmoteData);
	void SetBSDataBlender(FEmoteData&& EmoteData);

	/** Lerp first frames of a new Chat chunk from TempBs to avoid hard cuts at segment boundaries. */
	void StitchChatEmoteBoundary(FEmoteData& EmoteData) const;

	void SetBlendShapeValue(const TArray<float>& Data);
	void SetARKitBs(const TArray<float>& Data);
	void SetFLAMEBS(const TArray<float>& Data);

	UFUNCTION(BlueprintCallable, Category = "ATF")
	void Interrupt();

	void GetData(float DeltaTime, TArray<float>& OutResult);

	UFUNCTION(BlueprintCallable, Category = "ATF", meta = (DisplayName = "Init", AdvancedDisplay = "bInReplaceAnimInstance"))
	void InitFace(int32 FaceId, MotionType InMotionType, USkeletalMeshComponent* FaceMesh, bool bInReplaceAnimInstance = false);

	void Init(int32 FaceId, MotionType InMotionType, USkeletalMeshComponent* FaceMesh);
	void Init(int32 FaceId, MotionType InMotionType, USkeletalMeshComponent* Com, UClass* AnimClass);
	void Init(int32 FaceId, MotionType InMotionType, USkeletalMeshComponent* Com, UClass* AnimClass, bool bInReplaceAnimInstance);

	FName GetMappingName(FName Name) const;
	static FName MapMorphNameToARKit(FName MorphName);
	static FName ARKitEnumToString(int32 Index);
	static const TArray<FName>& GetARKitNames();
	static void GetMetaHumanCtrlAliases(FName ARKitName, TArray<FName>& OutNames);
	static int32 CountARKitMorphMatches(const USkeletalMesh* Mesh);

	/** Prefer a Face-named mesh with ARKit morphs (MetaHuman Face over Body / hair). */
	UFUNCTION(BlueprintPure, Category = "ATF")
	static USkeletalMeshComponent* FindFaceSkeletalMesh(AActor* Actor);

	/** First usable face mesh in the world, skipping IgnoreActor and the ATF sample actor. */
	UFUNCTION(BlueprintPure, Category = "ATF", meta = (WorldContext = "WorldContextObject"))
	static USkeletalMeshComponent* FindFaceSkeletalMeshInWorld(const UObject* WorldContextObject, AActor* IgnoreActor = nullptr);

	void ChangeState(EVHState State);
	void HandleIdle(float DeltaTime);
	void OnStartSpeech();
	void OnStopSpeech();
	void OnStopMotion();
	void RandomIdleHandle(float DeltaTime);

	bool IsPlayingEmote() const { return bIsPlayingEmote; }

	/** Game/anim-thread safe copy of the last ARKit frame (61 floats). */
	void CopyLatestARKitWeights(TArray<float>& OutWeights) const;

private:
	void BuildFlameNameCache();
	void SetPlayingEmote(bool bPlaying);
	bool NearlyEqualBs(const TArray<float>& A, const TArray<float>& B) const;
	UClass* ResolveDefaultAnimClass() const;
	AAudioToFaceManager* ResolveManager() const;
	void UpdateTickPolicy();
	void BindFaceMeshTick();
	void AddMorphTargetAlias(FName ARKitName, FName MorphName);
	void ApplyMorphValue(FName ARKitName, float Value);

	EATFDataType CurType = EATFDataType::None;
	float CurInterval = 0.f;
	bool bIsPlayingEmote = false;
	float CacheTime = 0.f;
	FATFExpressionBuffer BSData;
	FATFExpressionBuffer BSDataBlender;
	TMap<FName, TArray<FName>> ARKitMappingDic;
	int32 ChatBox = -1;
	float Weight = 1.f;
	int32 Dir = 1;
	TArray<float> TempBs;
	TArray<float> BlendScratch;
	TArray<float> LastAppliedBs;
	int32 PlayedFrameIndex = 0;
	/** Frames enqueued for the current Chat sequence (not reset on mid-stream append). */
	int32 ChatSessionFrameCount = 0;
	/** Seconds of Chat mouth already presented (sum of CurInterval steps). */
	float ChatEmotePlayedSeconds = 0.f;
	/** True while playing end-of-speech mouth decay (avoid re-Interrupt loop). */
	bool bChatClosingDecay = false;

	EVHState AvatarState = EVHState::Idle;
	float TempTime = 0.f;

	TArray<FName> FlamePositiveNames;
	TArray<FName> FlameNegativeNames;

	mutable FCriticalSection FaceCurveLock;
	TArray<float> LatestARKitWeights;

public:
	bool IsBusy = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF")
	MotionType CurMotionType = MotionType::ARKit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF")
	TObjectPtr<USkeletalMeshComponent> FaceCom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF")
	int32 Id = 0;

	/** If true, Init on BeginPlay. FaceCom is used when set; otherwise FindFaceSkeletalMesh(Owner). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF")
	bool bAutoInitOnBeginPlay = true;

	/**
	 * When true, replace FaceCom's AnimBP with UAudioToFaceAnim.
	 * Leave false for MetaHuman so Face AnimBP + ATF Face Sync can drive RigLogic.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|Anim")
	bool bReplaceAnimInstance = false;

	/** Random idle NPZ emotes when not speaking. */
	UPROPERTY(EditAnywhere, Category = "ATF|Idle")
	bool bEnableIdleEmote = false;

	UPROPERTY(EditAnywhere, Category = "ATF|Idle", meta = (EditCondition = "bEnableIdleEmote", ClampMin = "1.0"))
	float IdleIntervalSeconds = 5.f;

	/** When true (Chat), lips may not run more than ~0.35s ahead of audio. Frame advance is still DeltaTime-based (streaming-safe). */
	UPROPERTY(EditAnywhere, Category = "ATF|Audio")
	bool bSyncEmoteToAudioClock = false;

	/**
	 * Extra lead on top of Manager.LipSyncLeadSeconds (seconds).
	 * Increase if mouth still lags voice (try 0.05 ~ 0.12).
	 */
	UPROPERTY(EditAnywhere, Category = "ATF|Audio", meta = (EditCondition = "bSyncEmoteToAudioClock", ClampMin = "-0.25", ClampMax = "0.5"))
	float LipSyncLeadSeconds = 0.0f;

	UPROPERTY(EditAnywhere, Category = "ATF|Anim")
	TSoftClassPtr<UAudioToFaceAnim> FaceAnimClass;

	UPROPERTY(Transient)
	TObjectPtr<UAudioToFaceAnim> ATFAnim;
};
