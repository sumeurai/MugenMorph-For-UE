// Copyright 2026 Mugen3D. All Rights Reserved.

#include "Mugen3DMugenMorphInstallStat.h"

#include "MugenMorphApiSettings.h"
#include "Containers/Ticker.h"
#include "Dom/JsonObject.h"
#include "Engine/Engine.h"
#include "Misc/CoreDelegates.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/EngineVersion.h"
#include "Misc/Guid.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "SumeruMugenMorphProjectSettings.h"

namespace
{
	const TCHAR* EventPath = TEXT("v1/event-stats/increment");
	const TCHAR* ConfigSection = TEXT("Mugen3D.InstallStat");
	const TCHAR* ReportedKey = TEXT("Reported");
	const TCHAR* AnonIdKey = TEXT("AnonId");
	const TCHAR* DebugLogKey = TEXT("DebugLog");

	const TCHAR* BizType = TEXT("13");
	const TCHAR* EventType = TEXT("2");
	const TCHAR* EventKey = TEXT("sdk/atfue/install_success");
	const TCHAR* Product = TEXT("atf");
	const TCHAR* StatPackageName = TEXT("atf");
}

bool FMugen3DMugenMorphInstallStat::bStarted = false;
FDelegateHandle FMugen3DMugenMorphInstallStat::PostEngineInitHandle;

void FMugen3DMugenMorphInstallStat::Startup()
{
	if (IsRunningCommandlet() || FApp::IsUnattended())
	{
		return;
	}

	if (FParse::Param(FCommandLine::Get(), TEXT("unattended")))
	{
		return;
	}

	if (GEngine)
	{
		ScheduleTryReport();
		return;
	}

#if ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 8)
	PostEngineInitHandle = FCoreDelegates::GetOnPostEngineInit().AddStatic(&FMugen3DMugenMorphInstallStat::ScheduleTryReport);
#else
	PostEngineInitHandle = FCoreDelegates::OnPostEngineInit.AddStatic(&FMugen3DMugenMorphInstallStat::ScheduleTryReport);
#endif
}

void FMugen3DMugenMorphInstallStat::Shutdown()
{
	if (PostEngineInitHandle.IsValid())
	{
#if ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 8)
		FCoreDelegates::GetOnPostEngineInit().Remove(PostEngineInitHandle);
#else
		FCoreDelegates::OnPostEngineInit.Remove(PostEngineInitHandle);
#endif
		PostEngineInitHandle.Reset();
	}
}

void FMugen3DMugenMorphInstallStat::ScheduleTryReport()
{
	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([](float)
		{
			TryReport();
			return false;
		}),
		1.0f);
}

void FMugen3DMugenMorphInstallStat::TryReport()
{
	if (bStarted)
	{
		return;
	}

	bool bAlreadyReported = false;
	if (GConfig && GConfig->GetBool(ConfigSection, ReportedKey, bAlreadyReported, GEditorPerProjectIni) && bAlreadyReported)
	{
		LogIfDebug(TEXT("[Mugen3D] Install stat skipped: already reported"));
		return;
	}

	const USumeruMugenMorphProjectSettings* Settings = GetDefault<USumeruMugenMorphProjectSettings>();
	const FString BaseUrl = Settings ? Settings->BaseUrl : FString();
	if (BaseUrl.IsEmpty())
	{
		return;
	}

	bStarted = true;

	const FString Url = UMugenMorphApiSettings::CombineUrl(BaseUrl, EventPath);
	const FString Json = BuildRequestJson();
	LogIfDebug(FString::Printf(TEXT("[Mugen3D] Install stat URL: %s"), *Url));
	LogIfDebug(FString::Printf(TEXT("[Mugen3D] Install stat body: %s"), *Json));

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(Json);
	Request->OnProcessRequestComplete().BindStatic(&FMugen3DMugenMorphInstallStat::HandleResponse);
	if (!Request->ProcessRequest())
	{
		bStarted = false;
		LogIfDebug(TEXT("[Mugen3D] Install stat failed: could not start HTTP request"));
	}
}

void FMugen3DMugenMorphInstallStat::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	const int32 HttpCode = Response.IsValid() ? Response->GetResponseCode() : 0;
	const FString Body = Response.IsValid() ? Response->GetContentAsString() : FString();
	LogIfDebug(FString::Printf(
		TEXT("[Mugen3D] Install stat response: HTTP %d ok=%s\n%s"),
		HttpCode,
		bSucceeded ? TEXT("true") : TEXT("false"),
		*Body));

	if (!bSucceeded || !Response.IsValid() || HttpCode < 200 || HttpCode > 299)
	{
		bStarted = false;
		LogIfDebug(FString::Printf(TEXT("[Mugen3D] Install stat failed: HTTP %d"), HttpCode));
		return;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	int32 Code = 0;
	if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
	{
		double CodeNumber = 0.0;
		if (Root->TryGetNumberField(TEXT("code"), CodeNumber))
		{
			Code = static_cast<int32>(CodeNumber);
		}
	}

	if (Code != 200)
	{
		bStarted = false;
		const FString Msg = Root.IsValid() ? Root->GetStringField(TEXT("msg")) : TEXT("Response parse failed");
		LogIfDebug(FString::Printf(TEXT("[Mugen3D] Install stat failed: %s"), *Msg));
		return;
	}

	if (GConfig)
	{
		GConfig->SetBool(ConfigSection, ReportedKey, true, GEditorPerProjectIni);
		GConfig->Flush(false, GEditorPerProjectIni);
	}
}

FString FMugen3DMugenMorphInstallStat::BuildRequestJson()
{
	const USumeruMugenMorphProjectSettings* Settings = GetDefault<USumeruMugenMorphProjectSettings>();
	const FString EngineVersion = FEngineVersion::Current().ToString();
	const FString MachineId = FPlatformMisc::GetLoginId();
	const FString AnonId = GetOrCreateAnonId();
	const FString KeyId = Settings ? Settings->AccessKey : FString();
	const FString SdkVersion = GetSdkVersion();

	const bool bAllBlank = IsOptionalBlank(
		Product, StatPackageName, AnonId, SdkVersion, EngineVersion, MachineId, KeyId);

	TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	Object->SetStringField(TEXT("bizType"), BizType);
	Object->SetStringField(TEXT("eventType"), EventType);
	Object->SetStringField(TEXT("eventKey"), EventKey);
	Object->SetStringField(TEXT("product"), Product);
	Object->SetStringField(TEXT("packageName"), StatPackageName);
	Object->SetStringField(TEXT("version"), TEXT(""));
	Object->SetStringField(TEXT("fromPage"), TEXT(""));
	Object->SetStringField(TEXT("repo"), TEXT(""));
	Object->SetStringField(TEXT("anonId"), AnonId);
	Object->SetStringField(TEXT("sdkVersion"), SdkVersion);
	Object->SetStringField(TEXT("unityVersion"), EngineVersion);
	Object->SetStringField(TEXT("machineId"), MachineId);
	Object->SetNumberField(TEXT("userId"), 0);
	Object->SetStringField(TEXT("keyId"), KeyId);
	Object->SetStringField(TEXT("plan"), TEXT(""));
	Object->SetStringField(TEXT("apiName"), TEXT(""));
	Object->SetNumberField(TEXT("latencyMs"), 0);
	Object->SetNumberField(TEXT("callCount7d"), 0);
	Object->SetBoolField(TEXT("allBlank"), bAllBlank);

	FString Json;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Json);
	FJsonSerializer::Serialize(Object, Writer);
	return Json;
}

FString FMugen3DMugenMorphInstallStat::GetOrCreateAnonId()
{
	FString AnonId;
	if (GConfig)
	{
		GConfig->GetString(ConfigSection, AnonIdKey, AnonId, GEditorSettingsIni);
	}
	if (!AnonId.IsEmpty())
	{
		return AnonId;
	}

	AnonId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	if (GConfig)
	{
		GConfig->SetString(ConfigSection, AnonIdKey, *AnonId, GEditorSettingsIni);
		GConfig->Flush(false, GEditorSettingsIni);
	}
	return AnonId;
}

FString FMugen3DMugenMorphInstallStat::GetSdkVersion()
{
	if (const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("Mugen3D")))
	{
		return Plugin->GetDescriptor().VersionName;
	}
	return TEXT("1.0");
}

bool FMugen3DMugenMorphInstallStat::IsOptionalBlank(
	const FString& InProduct,
	const FString& InPackageName,
	const FString& AnonId,
	const FString& SdkVersion,
	const FString& EngineVersion,
	const FString& MachineId,
	const FString& KeyId)
{
	return InProduct.IsEmpty()
		&& InPackageName.IsEmpty()
		&& AnonId.IsEmpty()
		&& SdkVersion.IsEmpty()
		&& EngineVersion.IsEmpty()
		&& MachineId.IsEmpty()
		&& KeyId.IsEmpty();
}

void FMugen3DMugenMorphInstallStat::LogIfDebug(const FString& Message)
{
	bool bDebug = false;
	if (GConfig)
	{
		GConfig->GetBool(ConfigSection, DebugLogKey, bDebug, GEditorPerProjectIni);
	}
	if (bDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
	}
}
