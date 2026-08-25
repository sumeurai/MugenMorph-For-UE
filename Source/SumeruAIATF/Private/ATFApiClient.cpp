// Copyright SumeruAI. All Rights Reserved.

#include "ATFApiClient.h"

#include "ATFApiSettings.h"
#include "ATFAudioDecoder.h"
#include "ATFProtobuf.h"
#include "AudioToFace.h"
#include "Dom/JsonObject.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "SumeruATFLog.h"
#include "SumeruATFProjectSettings.h"

namespace ATFApiClientPrivate
{
	static uint16 ReadU16(const uint8* P)
	{
		return static_cast<uint16>(P[0] | (static_cast<uint16>(P[1]) << 8));
	}

	static uint32 ReadU32(const uint8* P)
	{
		return P[0]
			| (static_cast<uint32>(P[1]) << 8)
			| (static_cast<uint32>(P[2]) << 16)
			| (static_cast<uint32>(P[3]) << 24);
	}

	static int16 ReadI16(const uint8* P)
	{
		return static_cast<int16>(ReadU16(P));
	}

	static bool LooksLikeMp3(const TArray<uint8>& Raw)
	{
		if (Raw.Num() < 3)
		{
			return false;
		}
		if (Raw[0] == 'I' && Raw[1] == 'D' && Raw[2] == '3')
		{
			return true;
		}
		return Raw[0] == 0xFF && (Raw[1] & 0xE0) == 0xE0;
	}

	static bool DecodeWavPcm16Mono(const TArray<uint8>& Raw, TArray<uint8>& OutMono, int32& OutRate)
	{
		OutMono.Reset();
		OutRate = 0;
		if (Raw.Num() < 44)
		{
			return false;
		}
		if (FMemory::Memcmp(Raw.GetData(), "RIFF", 4) != 0 || FMemory::Memcmp(Raw.GetData() + 8, "WAVE", 4) != 0)
		{
			return false;
		}

		int32 Offset = 12;
		int32 Channels = 0;
		int32 Rate = 0;
		int32 Bits = 0;
		int32 AudioFormat = 0;
		int32 DataOff = INDEX_NONE;
		int32 DataSize = 0;

		while (Offset + 8 <= Raw.Num())
		{
			const uint8* ChunkId = Raw.GetData() + Offset;
			const uint32 ChunkSize = ReadU32(Raw.GetData() + Offset + 4);
			const int32 DataStart = Offset + 8;
			if (ChunkSize > static_cast<uint32>(Raw.Num() - DataStart))
			{
				break;
			}

			if (FMemory::Memcmp(ChunkId, "fmt ", 4) == 0)
			{
				if (ChunkSize < 16)
				{
					return false;
				}
				AudioFormat = ReadU16(Raw.GetData() + DataStart);
				Channels = ReadU16(Raw.GetData() + DataStart + 2);
				Rate = static_cast<int32>(ReadU32(Raw.GetData() + DataStart + 4));
				Bits = ReadU16(Raw.GetData() + DataStart + 14);
			}
			else if (FMemory::Memcmp(ChunkId, "data", 4) == 0)
			{
				DataOff = DataStart;
				DataSize = static_cast<int32>(ChunkSize);
				break;
			}

			Offset = DataStart + static_cast<int32>(ChunkSize);
			if (ChunkSize & 1)
			{
				++Offset;
			}
		}

		if (DataOff == INDEX_NONE || Channels <= 0 || Rate <= 0 || DataSize <= 0)
		{
			return false;
		}
		if (AudioFormat != 1 || Bits != 16)
		{
			return false;
		}

		const int32 BytesPerFrame = Channels * 2;
		if (BytesPerFrame <= 0)
		{
			return false;
		}
		const int32 Frames = DataSize / BytesPerFrame;
		OutMono.SetNumUninitialized(Frames * 2);
		int16* Dest = reinterpret_cast<int16*>(OutMono.GetData());
		const uint8* Src = Raw.GetData() + DataOff;
		for (int32 Index = 0; Index < Frames; ++Index)
		{
			int32 Acc = 0;
			for (int32 Channel = 0; Channel < Channels; ++Channel)
			{
				Acc += ReadI16(Src + Index * BytesPerFrame + Channel * 2);
			}
			Dest[Index] = static_cast<int16>(Acc / Channels);
		}
		OutRate = Rate;
		return OutMono.Num() > 0;
	}

	static bool DecodeRawPcm16Mono(const TArray<uint8>& Raw, TArray<uint8>& OutMono, int32& OutRate)
	{
		if (Raw.Num() < 2 || (Raw.Num() % 2) != 0)
		{
			return false;
		}
		OutMono = Raw;
		OutRate = 16000;
		return true;
	}

	static FString MakeTraceId()
	{
		return FGuid::NewGuid().ToString(EGuidFormats::Digits);
	}

	static FString WriteJsonObject(const TSharedRef<FJsonObject>& Object)
	{
		FString Json;
		const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
			TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Json);
		FJsonSerializer::Serialize(Object, Writer);
		return Json;
	}

	static bool TryReadJsonObject(const FString& Body, TSharedPtr<FJsonObject>& OutObject)
	{
		OutObject.Reset();
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
		return FJsonSerializer::Deserialize(Reader, OutObject) && OutObject.IsValid();
	}

	static int32 JsonInt(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field, int32 DefaultValue = 0)
	{
		double Number = DefaultValue;
		if (Object.IsValid() && Object->TryGetNumberField(Field, Number))
		{
			return static_cast<int32>(Number);
		}
		return DefaultValue;
	}

	static int64 JsonInt64(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field)
	{
		double Number = 0;
		if (Object.IsValid() && Object->TryGetNumberField(Field, Number))
		{
			return static_cast<int64>(Number);
		}
		return 0;
	}

	static bool ParseLoginRep(const FString& Body, FATFLoginRepData& Out)
	{
		Out = FATFLoginRepData();
		if (FJsonObjectConverter::JsonObjectStringToUStruct(Body, &Out, 0, 0)
			&& Out.Code == 200
			&& !Out.Data.AccessToken.IsEmpty())
		{
			return true;
		}

		TSharedPtr<FJsonObject> Root;
		if (!TryReadJsonObject(Body, Root))
		{
			return false;
		}

		Out.Code = JsonInt(Root, TEXT("code"));
		Root->TryGetStringField(TEXT("message"), Out.Message);
		const TSharedPtr<FJsonObject>* Data = nullptr;
		if (Root->TryGetObjectField(TEXT("data"), Data) && Data && (*Data).IsValid())
		{
			(*Data)->TryGetStringField(TEXT("accessToken"), Out.Data.AccessToken);
			Out.Data.ExpiresIn = JsonInt(*Data, TEXT("expiresIn"));
		}
		return Out.Code == 200 && !Out.Data.AccessToken.IsEmpty();
	}

	static bool ParseATFRep(const FString& Body, FATFRepData& Out)
	{
		Out = FATFRepData();
		if (FJsonObjectConverter::JsonObjectStringToUStruct(Body, &Out, 0, 0)
			&& (Out.Code == 0 || Out.Code == 200)
			&& (!Out.Data.EmoteKey.IsEmpty() || !Out.Data.AudioKey.IsEmpty()))
		{
			return true;
		}

		TSharedPtr<FJsonObject> Root;
		if (!TryReadJsonObject(Body, Root))
		{
			return false;
		}

		Out.Code = JsonInt(Root, TEXT("code"));
		Root->TryGetStringField(TEXT("message"), Out.Message);
		const TSharedPtr<FJsonObject>* Data = nullptr;
		if (Root->TryGetObjectField(TEXT("data"), Data) && Data && (*Data).IsValid())
		{
			Out.Data.Id = JsonInt64(*Data, TEXT("id"));
			(*Data)->TryGetStringField(TEXT("emoteKey"), Out.Data.EmoteKey);
			(*Data)->TryGetStringField(TEXT("audioKey"), Out.Data.AudioKey);
			double Fps = 0;
			if ((*Data)->TryGetNumberField(TEXT("fps"), Fps))
			{
				Out.Data.Fps = static_cast<float>(Fps);
			}
		}
		const bool bOkCode = (Out.Code == 0 || Out.Code == 200);
		return bOkCode && (!Out.Data.EmoteKey.IsEmpty() || !Out.Data.AudioKey.IsEmpty());
	}

	static bool LooksLikeJsonObject(const TArray<uint8>& Bytes)
	{
		for (int32 Index = 0; Index < Bytes.Num(); ++Index)
		{
			const uint8 Byte = Bytes[Index];
			if (Byte == ' ' || Byte == '\n' || Byte == '\r' || Byte == '\t')
			{
				continue;
			}
			return Byte == '{';
		}
		return false;
	}

	static FString BytesToUtf8String(const TArray<uint8>& Bytes)
	{
		if (Bytes.Num() <= 0)
		{
			return FString();
		}
		FUTF8ToTCHAR Convert(reinterpret_cast<const ANSICHAR*>(Bytes.GetData()), Bytes.Num());
		return FString(Convert.Length(), Convert.Get());
	}

	static void FillRepFromProtobuf(const FATFProtobufResult& Pb, FATFRepData& Out)
	{
		Out = FATFRepData();
		Out.Code = Pb.Code;
		Out.Message = Pb.Message;
		Out.Data.Fps = Pb.Fps;
		if (Pb.Blendshapes.Num() > 0)
		{
			Out.Data.EmoteKey = FBase64::Encode(Pb.Blendshapes);
		}
		if (Pb.Audio.Num() > 0)
		{
			Out.Data.AudioKey = FBase64::Encode(Pb.Audio);
		}
	}

	static FString ResolveWavPath(const FString& InPath)
	{
		if (InPath.IsEmpty())
		{
			return InPath;
		}
		if (FPaths::FileExists(InPath))
		{
			return InPath;
		}

		const FString Abs = FPaths::ConvertRelativePathToFull(InPath);
		if (FPaths::FileExists(Abs))
		{
			return Abs;
		}

		const FString FromProject = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / InPath);
		if (FPaths::FileExists(FromProject))
		{
			return FromProject;
		}

		return InPath;
	}
}

UATFApiClient::UATFApiClient()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UATFApiClient::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Cancel();
	Super::EndPlay(EndPlayReason);
}

void UATFApiClient::BeginDestroy()
{
	Cancel();
	Super::BeginDestroy();
}

void UATFApiClient::Cancel()
{
	ClearActiveRequest();
	PendingKind = EPendingKind::None;
	bBusy = false;
	bPostATFAfterLogin = false;
	PendingATFJson.Reset();
}

bool UATFApiClient::IgnoreIfBusy()
{
	if (!bBusy)
	{
		return false;
	}
	UE_LOG(LogSumeruATF, Warning, TEXT("ATF request ignored: a request is already in flight."));
	return true;
}

void UATFApiClient::Interrupt()
{
	Cancel();
	if (AAudioToFaceManager* Manager = AAudioToFaceManager::GetOrCreate(this))
	{
		Manager->ForceStopSpeech();
	}
}

bool UATFApiClient::ResolveCredentials(FString& OutAccessKey, FString& OutSecretKey) const
{
	OutAccessKey = AccessKeyOverride;
	OutSecretKey = SecretKeyOverride;
	if (OutAccessKey.IsEmpty() && Settings)
	{
		OutAccessKey = Settings->AccessKey;
	}
	if (OutSecretKey.IsEmpty() && Settings)
	{
		OutSecretKey = Settings->SecretKey;
	}

	const USumeruATFProjectSettings* ProjectSettings = GetDefault<USumeruATFProjectSettings>();
	if (OutAccessKey.IsEmpty() && ProjectSettings)
	{
		OutAccessKey = ProjectSettings->AccessKey;
	}
	if (OutSecretKey.IsEmpty() && ProjectSettings)
	{
		OutSecretKey = ProjectSettings->SecretKey;
	}
	return !OutAccessKey.IsEmpty() && !OutSecretKey.IsEmpty();
}

FString UATFApiClient::ResolveLoginUrl() const
{
	if (Settings)
	{
		return Settings->GetLoginUrl();
	}
	const USumeruATFProjectSettings* ProjectSettings = GetDefault<USumeruATFProjectSettings>();
	if (ProjectSettings)
	{
		return UATFApiSettings::CombineUrl(ProjectSettings->BaseUrl, ProjectSettings->LoginPath);
	}
	return UATFApiSettings::CombineUrl(TEXT("https://api.sumeruai.us/"), TEXT("v1/access/auth"));
}

FString UATFApiClient::ResolveATFMeshUrl() const
{
	if (Settings)
	{
		return Settings->GetATFMeshUrl();
	}
	const USumeruATFProjectSettings* ProjectSettings = GetDefault<USumeruATFProjectSettings>();
	if (ProjectSettings)
	{
		return UATFApiSettings::CombineUrl(ProjectSettings->BaseUrl, ProjectSettings->ATFMeshPath);
	}
	return UATFApiSettings::CombineUrl(TEXT("https://api.sumeruai.us/"), TEXT("v1/audio-to-face/offline-mesh"));
}

EATFApiTransport UATFApiClient::ResolveTransport() const
{
	if (Settings)
	{
		return Settings->ResolveTransport();
	}
	const USumeruATFProjectSettings* ProjectSettings = GetDefault<USumeruATFProjectSettings>();
	if (ProjectSettings && ProjectSettings->Transport != EATFApiTransport::Auto)
	{
		return ProjectSettings->Transport;
	}
	const FString Path = ProjectSettings ? ProjectSettings->ATFMeshPath : TEXT("v1/audio-to-face/offline-mesh");
	return Path.Contains(TEXT("offline"), ESearchCase::IgnoreCase)
		? EATFApiTransport::OfflineProtobuf
		: EATFApiTransport::MeshJson;
}

void UATFApiClient::ClearActiveRequest()
{
	if (ActiveRequest.IsValid())
	{
		ActiveRequest->OnProcessRequestComplete().Unbind();
		ActiveRequest->CancelRequest();
		ActiveRequest.Reset();
	}
}

bool UATFApiClient::StartPost(const FString& Url, const FString& JsonBody, bool bSendToken, EPendingKind Kind, bool bAcceptProtobuf)
{
	if (Url.IsEmpty())
	{
		Fail(TEXT("ATF API URL is empty."));
		return false;
	}

	ClearActiveRequest();
	PendingKind = Kind;
	bBusy = true;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	if (bAcceptProtobuf)
	{
		Request->SetHeader(TEXT("Accept"), TEXT("application/x-protobuf"));
	}
	Request->SetTimeout(TimeoutSeconds);
	if (bSendToken && !AccessToken.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), AccessToken);
	}
	Request->SetContentAsString(JsonBody);
	Request->OnProcessRequestComplete().BindUObject(this, &UATFApiClient::HandleHttpComplete);
	ActiveRequest = Request;

	UE_LOG(LogSumeruATF, Log, TEXT("ATF API POST %s acceptProtobuf=%d (%d chars)"),
		*Url, bAcceptProtobuf ? 1 : 0, JsonBody.Len());
	if (!Request->ProcessRequest())
	{
		ClearActiveRequest();
		Fail(TEXT("Failed to start HTTP request."));
		return false;
	}
	return true;
}

FString UATFApiClient::ToLoginJson(const FString& InAccessKey, const FString& InSecretKey)
{
	TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	Object->SetStringField(TEXT("accessKey"), InAccessKey);
	Object->SetStringField(TEXT("secretKey"), InSecretKey);
	return ATFApiClientPrivate::WriteJsonObject(Object);
}

FString UATFApiClient::ToATFReqJson(const FATFReqData& Request)
{
	TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	Object->SetStringField(TEXT("status"), Request.Status);
	Object->SetStringField(TEXT("dialogueBase64"), Request.DialogueBase64);
	Object->SetStringField(TEXT("lastDialogueBase64"), Request.LastDialogueBase64);
	Object->SetStringField(TEXT("traceId"), Request.TraceId);
	return ATFApiClientPrivate::WriteJsonObject(Object);
}

FString UATFApiClient::ToOfflineReqJson(const FATFOfflineReqData& Request)
{
	TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	Object->SetStringField(TEXT("traceId"), Request.TraceId);
	Object->SetStringField(TEXT("data"), Request.Data);
	return ATFApiClientPrivate::WriteJsonObject(Object);
}

bool UATFApiClient::BeginATFRequest(const FString& JsonBody, const FString& TraceId, const FString& DialogueBase64)
{
	PendingATFJson = JsonBody;
	LastTraceId = TraceId;
	LastDialogueBase64 = DialogueBase64;

	if (AccessToken.IsEmpty())
	{
		return LoginInternal(true);
	}

	const bool bProtobuf = ResolveTransport() == EATFApiTransport::OfflineProtobuf;
	return StartPost(ResolveATFMeshUrl(), JsonBody, true, EPendingKind::ATF, bProtobuf);
}

bool UATFApiClient::Login()
{
	return LoginInternal(false);
}

bool UATFApiClient::LoginInternal(bool bThenATF)
{
	if (IgnoreIfBusy())
	{
		return false;
	}

	FString Key, Secret;
	if (!ResolveCredentials(Key, Secret))
	{
		Fail(TEXT("AccessKey/SecretKey empty. Fill Project Settings → SumeruAI → ATF API, a Sumeru ATF API Settings asset, or client overrides."));
		return false;
	}

	bPostATFAfterLogin = bThenATF;
	return StartPost(ResolveLoginUrl(), ToLoginJson(Key, Secret), false, EPendingKind::Login, false);
}

bool UATFApiClient::RequestAudioToFace(const FATFReqData& Request)
{
	if (IgnoreIfBusy())
	{
		return false;
	}

	FATFReqData Copy = Request;
	if (Copy.Status.IsEmpty())
	{
		Copy.Status = TEXT("start");
	}
	if (Copy.TraceId.IsEmpty())
	{
		Copy.TraceId = ATFApiClientPrivate::MakeTraceId();
	}
	if (Copy.DialogueBase64.IsEmpty())
	{
		Fail(TEXT("dialogueBase64 is empty."));
		return false;
	}

	PendingATFRequest = Copy;

	if (ResolveTransport() == EATFApiTransport::OfflineProtobuf)
	{
		FATFOfflineReqData Offline;
		Offline.TraceId = Copy.TraceId;
		Offline.Data = Copy.DialogueBase64;
		return BeginATFRequest(ToOfflineReqJson(Offline), Copy.TraceId, Copy.DialogueBase64);
	}

	return BeginATFRequest(ToATFReqJson(Copy), Copy.TraceId, Copy.DialogueBase64);
}

bool UATFApiClient::RequestOfflineMesh(const FATFOfflineReqData& Request)
{
	if (IgnoreIfBusy())
	{
		return false;
	}

	FATFOfflineReqData Copy = Request;
	if (Copy.TraceId.IsEmpty())
	{
		Copy.TraceId = ATFApiClientPrivate::MakeTraceId();
	}
	if (Copy.Data.IsEmpty())
	{
		Fail(TEXT("data is empty."));
		return false;
	}

	PendingATFRequest = FATFReqData();
	PendingATFRequest.TraceId = Copy.TraceId;
	PendingATFRequest.DialogueBase64 = Copy.Data;
	PendingATFRequest.Status = TEXT("start");
	return BeginATFRequest(ToOfflineReqJson(Copy), Copy.TraceId, Copy.Data);
}

bool UATFApiClient::RequestFromWavFile(const FString& AbsoluteOrRelativePath, const FString& Status)
{
	const FString Path = ATFApiClientPrivate::ResolveWavPath(AbsoluteOrRelativePath);
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *Path) || Bytes.Num() <= 0)
	{
		Fail(FString::Printf(TEXT("Failed to read wav file: %s"), *Path));
		return false;
	}
	return RequestFromWavBytes(Bytes, Status);
}

bool UATFApiClient::RequestFromWavBytes(const TArray<uint8>& WavBytes, const FString& Status)
{
	if (WavBytes.Num() <= 0)
	{
		Fail(TEXT("Wav bytes are empty."));
		return false;
	}
	PendingFallbackWav = WavBytes;
	return RequestFromWavBase64(FBase64::Encode(WavBytes), Status);
}

bool UATFApiClient::RequestFromWavBase64(const FString& DialogueBase64, const FString& Status)
{
	FATFReqData Req;
	Req.Status = Status.IsEmpty() ? TEXT("start") : Status;
	Req.DialogueBase64 = DialogueBase64;
	if (!Req.Status.Equals(TEXT("start"), ESearchCase::IgnoreCase))
	{
		Req.LastDialogueBase64 = LastDialogueBase64;
	}
	Req.TraceId = ATFApiClientPrivate::MakeTraceId();
	return RequestAudioToFace(Req);
}

bool UATFApiClient::PlayFromAudio(const TArray<uint8>& WavBytes)
{
	return RequestFromWavBytes(WavBytes, TEXT("start"));
}

void UATFApiClient::PostPendingATFAfterLogin()
{
	bPostATFAfterLogin = false;
	const bool bProtobuf = ResolveTransport() == EATFApiTransport::OfflineProtobuf;
	StartPost(ResolveATFMeshUrl(), PendingATFJson, true, EPendingKind::ATF, bProtobuf);
}

void UATFApiClient::HandleHttpComplete(
	FHttpRequestPtr Request,
	FHttpResponsePtr Response,
	bool bSucceeded)
{
	ActiveRequest.Reset();
	const EPendingKind Kind = PendingKind;
	PendingKind = EPendingKind::None;

	const int32 HttpCode = Response.IsValid() ? Response->GetResponseCode() : 0;
	const TArray<uint8> BodyBytes = Response.IsValid() ? Response->GetContent() : TArray<uint8>();
	const FString Body = ATFApiClientPrivate::BytesToUtf8String(BodyBytes);

	if (!bSucceeded || !Response.IsValid())
	{
		Fail(FString::Printf(TEXT("HTTP request failed (code=%d)."), HttpCode));
		return;
	}

	if (HttpCode == 401 && Kind == EPendingKind::ATF && bRetryLoginOnAuthFail)
	{
		bRetryLoginOnAuthFail = false;
		AccessToken.Reset();
		bBusy = false;
		LoginInternal(true);
		return;
	}

	if (HttpCode < 200 || HttpCode >= 300)
	{
		Fail(FString::Printf(TEXT("HTTP %d: %s"), HttpCode, *Body.Left(256)));
		return;
	}

	if (Kind == EPendingKind::Login)
	{
		HandleLoginComplete(Body);
	}
	else if (Kind == EPendingKind::ATF)
	{
		HandleATFComplete(BodyBytes);
	}
	else
	{
		bBusy = false;
	}
}

void UATFApiClient::HandleLoginComplete(const FString& Body)
{
	FATFLoginRepData Rep;
	if (!ATFApiClientPrivate::ParseLoginRep(Body, Rep))
	{
		Fail(Rep.Message.IsEmpty()
			? FString::Printf(TEXT("Login parse/code failed: %s"), *Body.Left(256))
			: Rep.Message);
		return;
	}

	AccessToken = Rep.Data.AccessToken;
	if (AccessToken.IsEmpty())
	{
		Fail(TEXT("Login succeeded but accessToken is empty."));
		return;
	}

	UE_LOG(LogSumeruATF, Log, TEXT("ATF API login ok (expiresIn=%d)"), Rep.Data.ExpiresIn);
	OnLoginSucceeded.Broadcast(AccessToken);

	if (bPostATFAfterLogin && !PendingATFJson.IsEmpty())
	{
		PostPendingATFAfterLogin();
		return;
	}

	bPostATFAfterLogin = false;
	bBusy = false;
}

void UATFApiClient::HandleATFComplete(const TArray<uint8>& BodyBytes)
{
	bBusy = false;
	bRetryLoginOnAuthFail = true;

	FATFRepData Rep;
	TArray<uint8> BlendshapeBytes;
	TArray<uint8> AudioBytes;
	float Fps = 0.f;

	if (ATFApiClientPrivate::LooksLikeJsonObject(BodyBytes))
	{
		const FString Body = ATFApiClientPrivate::BytesToUtf8String(BodyBytes);
		if (!ATFApiClientPrivate::ParseATFRep(Body, Rep))
		{
			Fail(Rep.Message.IsEmpty()
				? FString::Printf(TEXT("ATF parse/code failed: %s"), *Body.Left(256))
				: Rep.Message);
			return;
		}
		Fps = Rep.Data.Fps;
		if (!Rep.Data.EmoteKey.IsEmpty())
		{
			FString Normalized = Rep.Data.EmoteKey;
			Normalized.ReplaceInline(TEXT("\n"), TEXT(""));
			Normalized.ReplaceInline(TEXT("\r"), TEXT(""));
			Normalized.ReplaceInline(TEXT(" "), TEXT(""));
			FBase64::Decode(Normalized, BlendshapeBytes);
		}
		if (!Rep.Data.AudioKey.IsEmpty())
		{
			FString Normalized = Rep.Data.AudioKey;
			Normalized.ReplaceInline(TEXT("\n"), TEXT(""));
			Normalized.ReplaceInline(TEXT("\r"), TEXT(""));
			Normalized.ReplaceInline(TEXT(" "), TEXT(""));
			FBase64::Decode(Normalized, AudioBytes);
		}
	}
	else
	{
		const FATFProtobufResult Pb = FATFProtobufParser::Parse(BodyBytes);
		UE_LOG(LogSumeruATF, Log,
			TEXT("ATF protobuf code=%d fps=%f frames=%d audio=%d blendshapes=%d"),
			Pb.Code, Pb.Fps, Pb.NumFrames, Pb.Audio.Num(), Pb.Blendshapes.Num());
		if (!Pb.IsSuccess())
		{
			Fail(Pb.Message.IsEmpty()
				? FString::Printf(TEXT("ATF protobuf failed code=%d (%d bytes)"), Pb.Code, BodyBytes.Num())
				: Pb.Message);
			return;
		}
		ATFApiClientPrivate::FillRepFromProtobuf(Pb, Rep);
		BlendshapeBytes = Pb.Blendshapes;
		AudioBytes = Pb.Audio;
		Fps = Pb.Fps;
	}

	if (bApplyToManager)
	{
		if (!ApplyRawAtfToManager(this, BlendshapeBytes, AudioBytes, Fps, FaceId, PendingFallbackWav))
		{
			Fail(TEXT("ATF response ok but failed to decode/apply blendshapes/audio."));
			return;
		}
	}

	OnRequestSucceeded.Broadcast(Rep, LastTraceId);
}

void UATFApiClient::Fail(const FString& Error)
{
	bBusy = false;
	PendingKind = EPendingKind::None;
	UE_LOG(LogSumeruATF, Error, TEXT("ATF API: %s"), *Error);
	OnRequestFailed.Broadcast(Error);
}

bool UATFApiClient::DecodeEmoteKey(const FString& EmoteKeyB64, float Fps, FEmoteData& OutEmote)
{
	OutEmote = FEmoteData();
	if (EmoteKeyB64.IsEmpty())
	{
		return false;
	}

	FString Normalized = EmoteKeyB64;
	Normalized.ReplaceInline(TEXT("\n"), TEXT(""));
	Normalized.ReplaceInline(TEXT("\r"), TEXT(""));
	Normalized.ReplaceInline(TEXT(" "), TEXT(""));

	TArray<uint8> Bytes;
	if (!FBase64::Decode(Normalized, Bytes))
	{
		return false;
	}
	return DecodeEmoteBytes(Bytes, Fps, OutEmote);
}

bool UATFApiClient::DecodeEmoteBytes(const TArray<uint8>& BlendshapeBytes, float Fps, FEmoteData& OutEmote)
{
	OutEmote = FEmoteData();
	if (BlendshapeBytes.Num() < 4 || (BlendshapeBytes.Num() % 4) != 0)
	{
		return false;
	}

	const int32 FloatCount = BlendshapeBytes.Num() / 4;
	TArray<float> Floats;
	Floats.SetNumUninitialized(FloatCount);
	FMemory::Memcpy(Floats.GetData(), BlendshapeBytes.GetData(), FloatCount * sizeof(float));

	int32 Coeffs = 61;
	if (FloatCount % 61 != 0)
	{
		if (FloatCount % 52 == 0)
		{
			Coeffs = 52;
		}
		else if (FloatCount % 51 == 0)
		{
			Coeffs = 51;
		}
		else
		{
			UE_LOG(LogSumeruATF, Error, TEXT("DecodeEmoteBytes: float count %d is not a multiple of 61/52/51"), FloatCount);
			return false;
		}
	}

	const int32 NumFrames = FloatCount / Coeffs;
	OutEmote.EmoteValues.Reserve(NumFrames);
	for (int32 Frame = 0; Frame < NumFrames; ++Frame)
	{
		FAudioExpression Expr;
		Expr.Data.Append(Floats.GetData() + Frame * Coeffs, Coeffs);
		OutEmote.EmoteValues.Add(MoveTemp(Expr));
	}

	OutEmote.Interval = (Fps > KINDA_SMALL_NUMBER) ? (1.f / Fps) : (1.f / 30.f);
	OutEmote.aTFDataType = EATFDataType::Chat;
	return OutEmote.EmoteValues.Num() > 0;
}

bool UATFApiClient::DecodeAudioKey(const FString& AudioKeyB64, TArray<uint8>& OutPcm16Mono, int32& OutSampleRate)
{
	OutPcm16Mono.Reset();
	OutSampleRate = 0;
	if (AudioKeyB64.IsEmpty())
	{
		return false;
	}

	TArray<uint8> Raw;
	if (!FBase64::Decode(AudioKeyB64, Raw) || Raw.Num() <= 0)
	{
		return false;
	}
	return DecodeAudioBytes(Raw, OutPcm16Mono, OutSampleRate);
}

bool UATFApiClient::DecodeAudioBytes(const TArray<uint8>& AudioBytes, TArray<uint8>& OutPcm16Mono, int32& OutSampleRate)
{
	OutPcm16Mono.Reset();
	OutSampleRate = 0;
	if (AudioBytes.Num() <= 0)
	{
		return false;
	}

	if (ATFApiClientPrivate::DecodeWavPcm16Mono(AudioBytes, OutPcm16Mono, OutSampleRate))
	{
		return true;
	}

	if (ATFApiClientPrivate::LooksLikeMp3(AudioBytes))
	{
		if (ATFAudioDecoder::DecodeMp3ToMonoInt16(AudioBytes, OutPcm16Mono, OutSampleRate))
		{
			return true;
		}
		UE_LOG(LogSumeruATF, Error, TEXT("DecodeAudioBytes: MP3 header detected but decode failed (bytes=%d)."), AudioBytes.Num());
		return false;
	}

	if (ATFAudioDecoder::DecodeMp3ToMonoInt16(AudioBytes, OutPcm16Mono, OutSampleRate))
	{
		UE_LOG(LogSumeruATF, Log, TEXT("DecodeAudioBytes: decoded MP3 without standard magic (bytes=%d rate=%d)."), AudioBytes.Num(), OutSampleRate);
		return true;
	}

	return ATFApiClientPrivate::DecodeRawPcm16Mono(AudioBytes, OutPcm16Mono, OutSampleRate);
}

bool UATFApiClient::ApplyResponseToManager(const UObject* WorldContextObject, const FATFRepData& Response, int32 InFaceId)
{
	TArray<uint8> Blendshapes;
	TArray<uint8> Audio;
	if (!Response.Data.EmoteKey.IsEmpty())
	{
		FBase64::Decode(Response.Data.EmoteKey, Blendshapes);
	}
	if (!Response.Data.AudioKey.IsEmpty())
	{
		FBase64::Decode(Response.Data.AudioKey, Audio);
	}
	TArray<uint8> EmptyFallback;
	return ApplyRawAtfToManager(WorldContextObject, Blendshapes, Audio, Response.Data.Fps, InFaceId, EmptyFallback);
}

bool UATFApiClient::ApplyRawAtfToManager(
	const UObject* WorldContextObject,
	const TArray<uint8>& BlendshapeBytes,
	const TArray<uint8>& AudioBytes,
	float Fps,
	int32 InFaceId,
	const TArray<uint8>& FallbackWavBytes)
{
	AAudioToFaceManager* Manager = AAudioToFaceManager::GetOrCreate(WorldContextObject);
	if (!Manager)
	{
		UE_LOG(LogSumeruATF, Error, TEXT("ApplyRawAtfToManager: AAudioToFaceManager missing."));
		return false;
	}

	FEmoteData Emote;
	const bool bHaveEmote = DecodeEmoteBytes(BlendshapeBytes, Fps, Emote);
	if (bHaveEmote)
	{
		Emote.FaceId = InFaceId;
	}

	TArray<uint8> Pcm;
	int32 SampleRate = 16000;
	bool bHaveAudio = DecodeAudioBytes(AudioBytes, Pcm, SampleRate);
	if (!bHaveAudio && FallbackWavBytes.Num() > 0)
	{
		UE_LOG(LogSumeruATF, Log, TEXT("ATF response has no audio, playing local wav %d bytes"), FallbackWavBytes.Num());
		bHaveAudio = DecodeAudioBytes(FallbackWavBytes, Pcm, SampleRate);
	}

	if (bHaveAudio && bHaveEmote)
	{
		Manager->EnqueueExternalSpeech(InFaceId, MoveTemp(Pcm), MoveTemp(Emote), SampleRate);
		return true;
	}
	if (bHaveEmote)
	{
		Manager->EnqueueExternalEmote(InFaceId, MoveTemp(Emote));
		return true;
	}
	if (bHaveAudio)
	{
		Manager->EnqueueExternalAudio(Pcm, SampleRate);
		return true;
	}
	return false;
}
