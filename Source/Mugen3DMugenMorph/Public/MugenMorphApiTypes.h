// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MugenMorphApiTypes.generated.h"

UENUM(BlueprintType)
enum class EMugenMorphApiTransport : uint8
{
	/** Infer from MugenMorphMeshPath: offline-mesh uses protobuf, otherwise mesh JSON. */
	Auto UMETA(DisplayName = "Auto"),
	/** Unity production: POST {traceId, data} with Accept: application/x-protobuf. */
	OfflineProtobuf UMETA(DisplayName = "Offline Mesh Protobuf"),
	/** Legacy: POST {status, dialogueBase64, lastDialogueBase64, traceId} JSON. */
	MeshJson UMETA(DisplayName = "Mesh JSON")
};

/** Unity LoginReqData */
USTRUCT(BlueprintType)
struct MUGEN3DMUGENMORPH_API FMugenMorphLoginReqData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API", meta = (JsonName = "accessKey"))
	FString AccessKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API", meta = (JsonName = "secretKey"))
	FString SecretKey;
};

/** Unity LoginRepBodyData */
USTRUCT(BlueprintType)
struct MUGEN3DMUGENMORPH_API FMugenMorphLoginRepBodyData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "MugenMorph|API", meta = (JsonName = "accessToken"))
	FString AccessToken;

	UPROPERTY(BlueprintReadOnly, Category = "MugenMorph|API", meta = (JsonName = "expiresIn"))
	int32 ExpiresIn = 0;
};

/** Unity LoginRepData */
USTRUCT(BlueprintType)
struct MUGEN3DMUGENMORPH_API FMugenMorphLoginRepData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "MugenMorph|API", meta = (JsonName = "code"))
	int32 Code = 0;

	UPROPERTY(BlueprintReadOnly, Category = "MugenMorph|API", meta = (JsonName = "message"))
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "MugenMorph|API", meta = (JsonName = "data"))
	FMugenMorphLoginRepBodyData Data;
};

/** Unity MugenMorphReqData for offline-mesh: { traceId, data }. */
USTRUCT(BlueprintType)
struct MUGEN3DMUGENMORPH_API FMugenMorphOfflineReqData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API", meta = (JsonName = "traceId"))
	FString TraceId;

	/** WAV (or PCM) bytes as base64 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API", meta = (JsonName = "data"))
	FString Data;
};

/** Legacy chunked mesh MugenMorph request (v1/audio-to-face/mesh). */
USTRUCT(BlueprintType)
struct MUGEN3DMUGENMORPH_API FMugenMorphReqData
{
	GENERATED_BODY()

	/** start / middle / end */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API", meta = (JsonName = "status"))
	FString Status = TEXT("start");

	/** WAV (or PCM) bytes as base64 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API", meta = (JsonName = "dialogueBase64"))
	FString DialogueBase64;

	/** Previous chunk audio, optional */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API", meta = (JsonName = "lastDialogueBase64"))
	FString LastDialogueBase64;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph|API", meta = (JsonName = "traceId"))
	FString TraceId;
};

/** Unity MugenMorphRepBodyData */
USTRUCT(BlueprintType)
struct MUGEN3DMUGENMORPH_API FMugenMorphRepBodyData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "MugenMorph|API", meta = (JsonName = "id"))
	int64 Id = 0;

	/** Blendshape float32 LE matrix, base64. Unity splits by 61 coeffs/frame. */
	UPROPERTY(BlueprintReadOnly, Category = "MugenMorph|API", meta = (JsonName = "emoteKey"))
	FString EmoteKey;

	/** WAV (preferred) or raw PCM16, base64 */
	UPROPERTY(BlueprintReadOnly, Category = "MugenMorph|API", meta = (JsonName = "audioKey"))
	FString AudioKey;

	UPROPERTY(BlueprintReadOnly, Category = "MugenMorph|API", meta = (JsonName = "fps"))
	float Fps = 0.f;
};

/** Unity MugenMorphRepData */
USTRUCT(BlueprintType)
struct MUGEN3DMUGENMORPH_API FMugenMorphRepData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "MugenMorph|API", meta = (JsonName = "code"))
	int32 Code = 0;

	UPROPERTY(BlueprintReadOnly, Category = "MugenMorph|API", meta = (JsonName = "message"))
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "MugenMorph|API", meta = (JsonName = "data"))
	FMugenMorphRepBodyData Data;
};
