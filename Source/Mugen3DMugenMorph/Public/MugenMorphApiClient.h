// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Http.h"
#include "MugenMorphApiTypes.h"
#include "MugenMorph_Type.h"
#include "MugenMorphApiClient.generated.h"

class UMugenMorphApiSettings;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMugenMorphApiLoginSucceeded, const FString&, AccessToken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMugenMorphApiRequestSucceeded, const FMugenMorphRepData&, Response, const FString&, TraceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMugenMorphApiRequestFailed, const FString&, ErrorMessage);

/**
 * Unity-compatible MugenMorph client:
 * Login (accessKey/secretKey) -> POST v1/audio-to-face/offline-mesh (protobuf)
 * with JSON mesh fallback (v1/audio-to-face/mesh).
 * Does not include TalkStream /talk.
 */
UCLASS(ClassGroup = (MugenMorph), meta = (BlueprintSpawnableComponent, DisplayName = "Sumeru MugenMorph API Client"))
class MUGEN3DMUGENMORPH_API UMugenMorphApiClient : public UActorComponent
{
	GENERATED_BODY()

public:
	UMugenMorphApiClient();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API")
	TObjectPtr<UMugenMorphApiSettings> Settings;

	/** If set, overrides Settings.AccessKey (quick test on the sample actor). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API")
	FString AccessKeyOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API")
	FString SecretKeyOverride;

	/** Push decoded audio+emote into AMugenMorphManager. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API")
	bool bApplyToManager = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API")
	int32 FaceId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API", meta = (ClampMin = "5.0"))
	float TimeoutSeconds = 120.f;

	UPROPERTY(BlueprintReadOnly, Category = "MugenMorph|API")
	FString AccessToken;

	UPROPERTY(BlueprintReadOnly, Category = "MugenMorph|API")
	FString LastTraceId;

	UPROPERTY(BlueprintReadOnly, Category = "MugenMorph|API")
	FString LastDialogueBase64;

	UPROPERTY(BlueprintAssignable, Category = "MugenMorph|API")
	FOnMugenMorphApiLoginSucceeded OnLoginSucceeded;

	UPROPERTY(BlueprintAssignable, Category = "MugenMorph|API")
	FOnMugenMorphApiRequestSucceeded OnRequestSucceeded;

	UPROPERTY(BlueprintAssignable, Category = "MugenMorph|API")
	FOnMugenMorphApiRequestFailed OnRequestFailed;

	UFUNCTION(BlueprintPure, Category = "MugenMorph|API")
	bool IsBusy() const { return bBusy; }

	UFUNCTION(BlueprintCallable, Category = "MugenMorph|API")
	void Cancel();

	/** Stop in-flight HTTP and fade current speech / mouth (Unity Interrupt). */
	UFUNCTION(BlueprintCallable, Category = "MugenMorph|API")
	void Interrupt();

	UFUNCTION(BlueprintCallable, Category = "MugenMorph|API")
	bool Login();

	/** POST MugenMorphReqData. Logs in first when AccessToken is empty. Uses resolved transport. */
	UFUNCTION(BlueprintCallable, Category = "MugenMorph|API")
	bool RequestMugenMorph(const FMugenMorphReqData& Request);

	/** Unity offline-mesh body {traceId, data}. */
	UFUNCTION(BlueprintCallable, Category = "MugenMorph|API")
	bool RequestOfflineMesh(const FMugenMorphOfflineReqData& Request);

	UFUNCTION(BlueprintCallable, Category = "MugenMorph|API")
	bool RequestFromWavFile(const FString& AbsoluteOrRelativePath, const FString& Status = TEXT("start"));

	UFUNCTION(BlueprintCallable, Category = "MugenMorph|API")
	bool RequestFromWavBytes(const TArray<uint8>& WavBytes, const FString& Status = TEXT("start"));

	UFUNCTION(BlueprintCallable, Category = "MugenMorph|API")
	bool RequestFromWavBase64(const FString& DialogueBase64, const FString& Status = TEXT("start"));

	/** Unity PlayFromAudio: login if needed, POST wav, apply to manager (local wav as audio fallback). */
	UFUNCTION(BlueprintCallable, Category = "MugenMorph|API")
	bool PlayFromAudio(const TArray<uint8>& WavBytes);

	UFUNCTION(BlueprintPure, Category = "MugenMorph|API")
	static bool DecodeEmoteKey(const FString& EmoteKeyB64, float Fps, FEmoteData& OutEmote);

	UFUNCTION(BlueprintPure, Category = "MugenMorph|API")
	static bool DecodeEmoteBytes(const TArray<uint8>& BlendshapeBytes, float Fps, FEmoteData& OutEmote);

	/** WAV, MP3, or raw PCM16. Output is mono PCM16. */
	UFUNCTION(BlueprintPure, Category = "MugenMorph|API")
	static bool DecodeAudioKey(const FString& AudioKeyB64, TArray<uint8>& OutPcm16Mono, int32& OutSampleRate);

	UFUNCTION(BlueprintPure, Category = "MugenMorph|API")
	static bool DecodeAudioBytes(const TArray<uint8>& AudioBytes, TArray<uint8>& OutPcm16Mono, int32& OutSampleRate);

	UFUNCTION(BlueprintCallable, Category = "MugenMorph|API", meta = (WorldContext = "WorldContextObject"))
	static bool ApplyResponseToManager(const UObject* WorldContextObject, const FMugenMorphRepData& Response, int32 InFaceId = 0);

	UFUNCTION(BlueprintCallable, Category = "MugenMorph|API", meta = (WorldContext = "WorldContextObject"))
	static bool ApplyRawAtfToManager(
		const UObject* WorldContextObject,
		const TArray<uint8>& BlendshapeBytes,
		const TArray<uint8>& AudioBytes,
		float Fps,
		int32 InFaceId,
		const TArray<uint8>& FallbackWavBytes);

private:
	enum class EPendingKind : uint8
	{
		None,
		Login,
		MugenMorph
	};

	bool ResolveCredentials(FString& OutAccessKey, FString& OutSecretKey) const;
	FString ResolveLoginUrl() const;
	FString ResolveMugenMorphMeshUrl() const;
	EMugenMorphApiTransport ResolveTransport() const;
	bool LoginInternal(bool bThenMugenMorph);
	bool IgnoreIfBusy();
	bool StartPost(const FString& Url, const FString& JsonBody, bool bSendToken, EPendingKind Kind, bool bAcceptProtobuf);
	bool BeginMugenMorphRequest(const FString& JsonBody, const FString& TraceId, const FString& DialogueBase64);
	void HandleHttpComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void HandleLoginComplete(const FString& Body);
	void HandleMugenMorphComplete(const TArray<uint8>& BodyBytes);
	void Fail(const FString& Error);
	void ClearActiveRequest();
	void PostPendingMugenMorphAfterLogin();
	static FString ToLoginJson(const FString& InAccessKey, const FString& InSecretKey);
	static FString ToMugenMorphReqJson(const FMugenMorphReqData& Request);
	static FString ToOfflineReqJson(const FMugenMorphOfflineReqData& Request);

	bool bBusy = false;
	bool bPostMugenMorphAfterLogin = false;
	bool bRetryLoginOnAuthFail = true;
	EPendingKind PendingKind = EPendingKind::None;
	FMugenMorphReqData PendingMugenMorphRequest;
	FString PendingMugenMorphJson;
	TArray<uint8> PendingFallbackWav;
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> ActiveRequest;
};
