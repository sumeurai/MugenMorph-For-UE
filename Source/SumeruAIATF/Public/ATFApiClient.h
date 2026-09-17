// Copyright 2026 SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Http.h"
#include "ATFApiTypes.h"
#include "ATF_Type.h"
#include "ATFApiClient.generated.h"

class UATFApiSettings;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnATFApiLoginSucceeded, const FString&, AccessToken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnATFApiRequestSucceeded, const FATFRepData&, Response, const FString&, TraceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnATFApiRequestFailed, const FString&, ErrorMessage);

/**
 * Unity-compatible ATF client:
 * Login (accessKey/secretKey) -> POST v1/audio-to-face/offline-mesh (protobuf)
 * with JSON mesh fallback (v1/audio-to-face/mesh).
 * Does not include TalkStream /talk.
 */
UCLASS(ClassGroup = (ATF), meta = (BlueprintSpawnableComponent, DisplayName = "Sumeru ATF API Client"))
class SUMERUAIATF_API UATFApiClient : public UActorComponent
{
	GENERATED_BODY()

public:
	UATFApiClient();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API")
	TObjectPtr<UATFApiSettings> Settings;

	/** If set, overrides Settings.AccessKey (quick test on the sample actor). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API")
	FString AccessKeyOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API")
	FString SecretKeyOverride;

	/** Push decoded audio+emote into AAudioToFaceManager. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API")
	bool bApplyToManager = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API")
	int32 FaceId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API", meta = (ClampMin = "5.0"))
	float TimeoutSeconds = 120.f;

	UPROPERTY(BlueprintReadOnly, Category = "ATF|API")
	FString AccessToken;

	UPROPERTY(BlueprintReadOnly, Category = "ATF|API")
	FString LastTraceId;

	UPROPERTY(BlueprintReadOnly, Category = "ATF|API")
	FString LastDialogueBase64;

	UPROPERTY(BlueprintAssignable, Category = "ATF|API")
	FOnATFApiLoginSucceeded OnLoginSucceeded;

	UPROPERTY(BlueprintAssignable, Category = "ATF|API")
	FOnATFApiRequestSucceeded OnRequestSucceeded;

	UPROPERTY(BlueprintAssignable, Category = "ATF|API")
	FOnATFApiRequestFailed OnRequestFailed;

	UFUNCTION(BlueprintPure, Category = "ATF|API")
	bool IsBusy() const { return bBusy; }

	UFUNCTION(BlueprintCallable, Category = "ATF|API")
	void Cancel();

	/** Stop in-flight HTTP and fade current speech / mouth (Unity Interrupt). */
	UFUNCTION(BlueprintCallable, Category = "ATF|API")
	void Interrupt();

	UFUNCTION(BlueprintCallable, Category = "ATF|API")
	bool Login();

	/** POST ATFReqData. Logs in first when AccessToken is empty. Uses resolved transport. */
	UFUNCTION(BlueprintCallable, Category = "ATF|API")
	bool RequestAudioToFace(const FATFReqData& Request);

	/** Unity offline-mesh body {traceId, data}. */
	UFUNCTION(BlueprintCallable, Category = "ATF|API")
	bool RequestOfflineMesh(const FATFOfflineReqData& Request);

	UFUNCTION(BlueprintCallable, Category = "ATF|API")
	bool RequestFromWavFile(const FString& AbsoluteOrRelativePath, const FString& Status = TEXT("start"));

	UFUNCTION(BlueprintCallable, Category = "ATF|API")
	bool RequestFromWavBytes(const TArray<uint8>& WavBytes, const FString& Status = TEXT("start"));

	UFUNCTION(BlueprintCallable, Category = "ATF|API")
	bool RequestFromWavBase64(const FString& DialogueBase64, const FString& Status = TEXT("start"));

	/** Unity PlayFromAudio: login if needed, POST wav, apply to manager (local wav as audio fallback). */
	UFUNCTION(BlueprintCallable, Category = "ATF|API")
	bool PlayFromAudio(const TArray<uint8>& WavBytes);

	UFUNCTION(BlueprintPure, Category = "ATF|API")
	static bool DecodeEmoteKey(const FString& EmoteKeyB64, float Fps, FEmoteData& OutEmote);

	UFUNCTION(BlueprintPure, Category = "ATF|API")
	static bool DecodeEmoteBytes(const TArray<uint8>& BlendshapeBytes, float Fps, FEmoteData& OutEmote);

	/** WAV, MP3, or raw PCM16. Output is mono PCM16. */
	UFUNCTION(BlueprintPure, Category = "ATF|API")
	static bool DecodeAudioKey(const FString& AudioKeyB64, TArray<uint8>& OutPcm16Mono, int32& OutSampleRate);

	UFUNCTION(BlueprintPure, Category = "ATF|API")
	static bool DecodeAudioBytes(const TArray<uint8>& AudioBytes, TArray<uint8>& OutPcm16Mono, int32& OutSampleRate);

	UFUNCTION(BlueprintCallable, Category = "ATF|API", meta = (WorldContext = "WorldContextObject"))
	static bool ApplyResponseToManager(const UObject* WorldContextObject, const FATFRepData& Response, int32 InFaceId = 0);

	UFUNCTION(BlueprintCallable, Category = "ATF|API", meta = (WorldContext = "WorldContextObject"))
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
		ATF
	};

	bool ResolveCredentials(FString& OutAccessKey, FString& OutSecretKey) const;
	FString ResolveLoginUrl() const;
	FString ResolveATFMeshUrl() const;
	EATFApiTransport ResolveTransport() const;
	bool LoginInternal(bool bThenATF);
	bool IgnoreIfBusy();
	bool StartPost(const FString& Url, const FString& JsonBody, bool bSendToken, EPendingKind Kind, bool bAcceptProtobuf);
	bool BeginATFRequest(const FString& JsonBody, const FString& TraceId, const FString& DialogueBase64);
	void HandleHttpComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);
	void HandleLoginComplete(const FString& Body);
	void HandleATFComplete(const TArray<uint8>& BodyBytes);
	void Fail(const FString& Error);
	void ClearActiveRequest();
	void PostPendingATFAfterLogin();
	static FString ToLoginJson(const FString& InAccessKey, const FString& InSecretKey);
	static FString ToATFReqJson(const FATFReqData& Request);
	static FString ToOfflineReqJson(const FATFOfflineReqData& Request);

	bool bBusy = false;
	bool bPostATFAfterLogin = false;
	bool bRetryLoginOnAuthFail = true;
	EPendingKind PendingKind = EPendingKind::None;
	FATFReqData PendingATFRequest;
	FString PendingATFJson;
	TArray<uint8> PendingFallbackWav;
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> ActiveRequest;
};
