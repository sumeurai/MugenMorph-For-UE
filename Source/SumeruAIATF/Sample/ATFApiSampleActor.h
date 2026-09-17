// Copyright 2026 SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ATFApiTypes.h"
#include "ATFApiSampleActor.generated.h"

class UATFApiClient;
class USkeletalMeshComponent;
class UATFComponent;
class FATFAudioRecord;

/**
 * Sample-only actor (not a public SDK header).
 * Login, overlay UI, PlayFromAudio, mic record. Mirrors Unity AudioToFaceSample.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Sumeru ATF API Sample"))
class SUMERUAIATF_API AATFApiSampleActor : public AActor
{
	GENERATED_BODY()

public:
	AATFApiSampleActor();
	virtual ~AATFApiSampleActor() override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ATF|API")
	TObjectPtr<UATFApiClient> Client;

	/** Last WAV chosen via Browse (J). Empty until a file is selected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|Sample")
	FString WavFilePath;

	/** start / middle / end — only used for legacy mesh JSON transport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|Sample")
	FString Status = TEXT("start");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|Sample")
	bool bLoginOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|Sample")
	bool bShowOverlay = true;

	/** Optional face mesh. If empty, the first MetaHuman / ARKit face in the level is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|Sample")
	TObjectPtr<USkeletalMeshComponent> FaceMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|Sample")
	int32 FaceId = 0;

	/** Unity AudioRecord maxLengthSeconds (default 300). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|Sample")
	int32 MaxRecordSeconds = 300;

	UFUNCTION(BlueprintCallable, Category = "ATF|Sample")
	bool RequestFromWavFile();

	UFUNCTION(BlueprintCallable, Category = "ATF|Sample")
	bool RequestFromWavFilePath(const FString& AbsoluteOrRelativePath);

	UFUNCTION(BlueprintCallable, Category = "ATF|Sample")
	bool BrowseAndPlayWav();

	UFUNCTION(BlueprintCallable, Category = "ATF|Sample")
	void InterruptPlayback();

	/** Unity AudioToFaceSample.StartRecord. */
	UFUNCTION(BlueprintCallable, Category = "ATF|Sample")
	void StartRecord();

	/** Unity AudioToFaceSample.StopRecord: encode 16 kHz WAV then PlayFromAudio. */
	UFUNCTION(BlueprintCallable, Category = "ATF|Sample")
	void StopRecord();

	UFUNCTION(BlueprintPure, Category = "ATF|Sample")
	bool IsRecording() const;

private:
	UFUNCTION()
	void HandleLoginSucceeded(const FString& AccessToken);

	UFUNCTION()
	void HandleRequestSucceeded(const FATFRepData& Response, const FString& TraceId);

	UFUNCTION()
	void HandleRequestFailed(const FString& ErrorMessage);

	void EnsureRuntime();
	void BindClientEvents();
	void BindInput();
	void ShowOverlay();
	void HideOverlay();
	void SetStatus(const FString& Message);
	bool HasCredentials() const;
	UATFComponent* EnsureFaceComponent() const;
	void InputBrowseWav();
	void InputInterrupt();
	void InputToggleRecord();

	FString StatusMessage;
	FATFAudioRecord* Recorder = nullptr;
};
