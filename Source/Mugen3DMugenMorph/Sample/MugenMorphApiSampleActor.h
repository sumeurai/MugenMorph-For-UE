// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MugenMorphApiTypes.h"
#include "MugenMorphApiSampleActor.generated.h"

class UMugenMorphApiClient;
class USkeletalMeshComponent;
class UMugenMorphComponent;
class FMugenMorphAudioRecord;

/**
 * Sample-only actor (not a public SDK header).
 * Login, overlay UI, PlayFromAudio, mic record. Mirrors Unity MugenMorphSample.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Mugen3D MugenMorph API Sample"))
class MUGEN3DMUGENMORPH_API AMugenMorphApiSampleActor : public AActor
{
	GENERATED_BODY()

public:
	AMugenMorphApiSampleActor();
	virtual ~AMugenMorphApiSampleActor() override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MugenMorph|API")
	TObjectPtr<UMugenMorphApiClient> Client;

	/** Last WAV chosen via Browse (J). Empty until a file is selected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|Sample")
	FString WavFilePath;

	/** start / middle / end — only used for legacy mesh JSON transport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|Sample")
	FString Status = TEXT("start");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|Sample")
	bool bLoginOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|Sample")
	bool bShowOverlay = true;

	/** Optional face mesh. If empty, the first MetaHuman / ARKit face in the level is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|Sample")
	TObjectPtr<USkeletalMeshComponent> FaceMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|Sample")
	int32 FaceId = 0;

	/** Unity AudioRecord maxLengthSeconds (default 300). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|Sample")
	int32 MaxRecordSeconds = 300;

	UFUNCTION(BlueprintCallable, Category = "MugenMorph|Sample")
	bool RequestFromWavFile();

	UFUNCTION(BlueprintCallable, Category = "MugenMorph|Sample")
	bool RequestFromWavFilePath(const FString& AbsoluteOrRelativePath);

	UFUNCTION(BlueprintCallable, Category = "MugenMorph|Sample")
	bool BrowseAndPlayWav();

	UFUNCTION(BlueprintCallable, Category = "MugenMorph|Sample")
	void InterruptPlayback();

	/** Unity MugenMorphSample.StartRecord. */
	UFUNCTION(BlueprintCallable, Category = "MugenMorph|Sample")
	void StartRecord();

	/** Unity MugenMorphSample.StopRecord: encode 16 kHz WAV then PlayFromAudio. */
	UFUNCTION(BlueprintCallable, Category = "MugenMorph|Sample")
	void StopRecord();

	UFUNCTION(BlueprintPure, Category = "MugenMorph|Sample")
	bool IsRecording() const;

private:
	UFUNCTION()
	void HandleLoginSucceeded(const FString& AccessToken);

	UFUNCTION()
	void HandleRequestSucceeded(const FMugenMorphRepData& Response, const FString& TraceId);

	UFUNCTION()
	void HandleRequestFailed(const FString& ErrorMessage);

	void EnsureRuntime();
	void BindClientEvents();
	void BindInput();
	void ShowOverlay();
	void HideOverlay();
	void SetStatus(const FString& Message);
	bool HasCredentials() const;
	UMugenMorphComponent* EnsureFaceComponent() const;
	void InputBrowseWav();
	void InputInterrupt();
	void InputToggleRecord();

	FString StatusMessage;
	FMugenMorphAudioRecord* Recorder = nullptr;
};
