// Copyright 2026 SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ATFApiTypes.generated.h"

UENUM(BlueprintType)
enum class EATFApiTransport : uint8
{
	/** Infer from ATFMeshPath: offline-mesh uses protobuf, otherwise mesh JSON. */
	Auto UMETA(DisplayName = "Auto"),
	/** Unity production: POST {traceId, data} with Accept: application/x-protobuf. */
	OfflineProtobuf UMETA(DisplayName = "Offline Mesh Protobuf"),
	/** Legacy: POST {status, dialogueBase64, lastDialogueBase64, traceId} JSON. */
	MeshJson UMETA(DisplayName = "Mesh JSON")
};

/** Unity LoginReqData */
USTRUCT(BlueprintType)
struct SUMERUAIATF_API FATFLoginReqData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API", meta = (JsonName = "accessKey"))
	FString AccessKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API", meta = (JsonName = "secretKey"))
	FString SecretKey;
};

/** Unity LoginRepBodyData */
USTRUCT(BlueprintType)
struct SUMERUAIATF_API FATFLoginRepBodyData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ATF|API", meta = (JsonName = "accessToken"))
	FString AccessToken;

	UPROPERTY(BlueprintReadOnly, Category = "ATF|API", meta = (JsonName = "expiresIn"))
	int32 ExpiresIn = 0;
};

/** Unity LoginRepData */
USTRUCT(BlueprintType)
struct SUMERUAIATF_API FATFLoginRepData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ATF|API", meta = (JsonName = "code"))
	int32 Code = 0;

	UPROPERTY(BlueprintReadOnly, Category = "ATF|API", meta = (JsonName = "message"))
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "ATF|API", meta = (JsonName = "data"))
	FATFLoginRepBodyData Data;
};

/** Unity ATFReqData for offline-mesh: { traceId, data }. */
USTRUCT(BlueprintType)
struct SUMERUAIATF_API FATFOfflineReqData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API", meta = (JsonName = "traceId"))
	FString TraceId;

	/** WAV (or PCM) bytes as base64 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API", meta = (JsonName = "data"))
	FString Data;
};

/** Legacy chunked mesh ATF request (v1/audio-to-face/mesh). */
USTRUCT(BlueprintType)
struct SUMERUAIATF_API FATFReqData
{
	GENERATED_BODY()

	/** start / middle / end */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API", meta = (JsonName = "status"))
	FString Status = TEXT("start");

	/** WAV (or PCM) bytes as base64 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API", meta = (JsonName = "dialogueBase64"))
	FString DialogueBase64;

	/** Previous chunk audio, optional */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API", meta = (JsonName = "lastDialogueBase64"))
	FString LastDialogueBase64;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF|API", meta = (JsonName = "traceId"))
	FString TraceId;
};

/** Unity ATFRepBodyData */
USTRUCT(BlueprintType)
struct SUMERUAIATF_API FATFRepBodyData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ATF|API", meta = (JsonName = "id"))
	int64 Id = 0;

	/** Blendshape float32 LE matrix, base64. Unity splits by 61 coeffs/frame. */
	UPROPERTY(BlueprintReadOnly, Category = "ATF|API", meta = (JsonName = "emoteKey"))
	FString EmoteKey;

	/** WAV (preferred) or raw PCM16, base64 */
	UPROPERTY(BlueprintReadOnly, Category = "ATF|API", meta = (JsonName = "audioKey"))
	FString AudioKey;

	UPROPERTY(BlueprintReadOnly, Category = "ATF|API", meta = (JsonName = "fps"))
	float Fps = 0.f;
};

/** Unity ATFRepData */
USTRUCT(BlueprintType)
struct SUMERUAIATF_API FATFRepData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ATF|API", meta = (JsonName = "code"))
	int32 Code = 0;

	UPROPERTY(BlueprintReadOnly, Category = "ATF|API", meta = (JsonName = "message"))
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "ATF|API", meta = (JsonName = "data"))
	FATFRepBodyData Data;
};
