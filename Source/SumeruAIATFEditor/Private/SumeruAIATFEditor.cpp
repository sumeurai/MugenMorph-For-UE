// Copyright 2026 SumeruAI. All Rights Reserved.

#include "SumeruAIATFEditor.h"

#include "AssetRegistry/AssetData.h"
#include "ATFApiSampleActor.h" // Source/SumeruAIATF/Sample
#include "AudioToFace.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "ContentBrowserMenuContexts.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextRenderActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/FileManager.h"
#include "ISettingsModule.h"
#include "LevelEditor.h"
#include "Misc/MessageDialog.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "SumeruAIATFInstallStat.h"
#include "SumeruAIATFSetup.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FSumeruAIATFEditorModule"

namespace
{
	template <typename T>
	T* FindFirstActor(UWorld* World)
	{
		for (TActorIterator<T> It(World); It; ++It)
		{
			return *It;
		}
		return nullptr;
	}

	bool EnsureBasicLevelEnvironment(UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		bool bChanged = false;
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ADirectionalLight* Sun = FindFirstActor<ADirectionalLight>(World);
		if (!Sun)
		{
			Sun = World->SpawnActor<ADirectionalLight>(FVector(0.f, 0.f, 400.f), FRotator(-46.f, 0.f, 0.f), Params);
			bChanged = true;
		}
		if (Sun)
		{
			if (UDirectionalLightComponent* Light = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
			{
				Light->Modify();
				Light->SetAtmosphereSunLight(true);
				if (Light->Intensity < 1.f)
				{
					Light->SetIntensity(10.f);
				}
			}
		}

		if (!FindFirstActor<ASkyAtmosphere>(World))
		{
			World->SpawnActor<ASkyAtmosphere>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
			bChanged = true;
		}

		ASkyLight* SkyLight = FindFirstActor<ASkyLight>(World);
		if (!SkyLight)
		{
			SkyLight = World->SpawnActor<ASkyLight>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
			bChanged = true;
		}
		if (SkyLight)
		{
			if (USkyLightComponent* Sky = Cast<USkyLightComponent>(SkyLight->GetLightComponent()))
			{
				Sky->Modify();
				Sky->SetMobility(EComponentMobility::Movable);
				Sky->SetRealTimeCaptureEnabled(true);
				Sky->RecaptureSky();
			}
		}

		if (!FindFirstActor<AExponentialHeightFog>(World))
		{
			if (AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(
				FVector(0.f, 0.f, 50.f), FRotator::ZeroRotator, Params))
			{
				if (UExponentialHeightFogComponent* FogComp = Fog->GetComponent())
				{
					FogComp->SetFogDensity(0.02f);
				}
			}
			bChanged = true;
		}

		if (!FindFirstActor<AVolumetricCloud>(World))
		{
			World->SpawnActor<AVolumetricCloud>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
			bChanged = true;
		}

		if (!FindFirstActor<AStaticMeshActor>(World))
		{
			UStaticMesh* FloorMesh = LoadObject<UStaticMesh>(
				nullptr, TEXT("/Engine/MapTemplates/SM_Template_Map_Floor.SM_Template_Map_Floor"));
			if (!FloorMesh)
			{
				FloorMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
			}
			if (FloorMesh)
			{
				if (AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(
					FVector::ZeroVector, FRotator::ZeroRotator, Params))
				{
					Floor->GetStaticMeshComponent()->SetStaticMesh(FloorMesh);
					Floor->SetMobility(EComponentMobility::Static);
					if (FloorMesh->GetName().Contains(TEXT("Plane")))
					{
						Floor->SetActorScale3D(FVector(50.f, 50.f, 1.f));
					}
				}
			}
			bChanged = true;
		}

		if (!FindFirstActor<APlayerStart>(World))
		{
			World->SpawnActor<APlayerStart>(FVector(-400.f, 0.f, 92.f), FRotator::ZeroRotator, Params);
			bChanged = true;
		}

		return bChanged;
	}

	bool EnsureDemoActors(UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		bool bChanged = false;
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (!FindFirstActor<AAudioToFaceManager>(World))
		{
			World->SpawnActor<AAudioToFaceManager>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
			bChanged = true;
		}
		if (!FindFirstActor<AATFApiSampleActor>(World))
		{
			World->SpawnActor<AATFApiSampleActor>(FVector(0.f, 0.f, 80.f), FRotator::ZeroRotator, Params);
			bChanged = true;
		}
		if (!FindFirstActor<ATextRenderActor>(World))
		{
			if (ATextRenderActor* Hint = World->SpawnActor<ATextRenderActor>(
				FVector(80.f, 0.f, 180.f), FRotator(0.f, 180.f, 0.f), Params))
			{
				if (UTextRenderComponent* Text = Hint->GetTextRender())
				{
					Text->SetText(FText::FromString(
						TEXT("SumeruAI ATF Demo\nProject Settings -> SumeruAI -> ATF API\nImport MetaHuman, select it, SumeruAI -> Setup ATF\nPIE: P play  J browse  I interrupt")));
					Text->SetHorizontalAlignment(EHTA_Center);
					Text->SetWorldSize(18.f);
				}
			}
			bChanged = true;
		}

		return bChanged;
	}
}

void FSumeruAIATFEditorModule::StartupModule()
{
	FSumeruAIATFInstallStat::Startup();
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSumeruAIATFEditorModule::RegisterMenus));
}

void FSumeruAIATFEditorModule::ShutdownModule()
{
	FSumeruAIATFInstallStat::Shutdown();
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

void FSumeruAIATFEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* MainMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu");
	FToolMenuSection& Section = MainMenu->FindOrAddSection("SumeruAI");
	Section.AddSubMenu(
		"SumeruAI",
		LOCTEXT("SumeruAIMenu", "SumeruAI"),
		LOCTEXT("SumeruAIMenuTooltip", "SumeruAI Audio-to-Face"),
		FNewToolMenuDelegate::CreateLambda([](UToolMenu* InMenu)
		{
			FToolMenuSection& AtfSection = InMenu->AddSection("ATF", LOCTEXT("ATFSection", "Audio to Face"));
			AtfSection.AddMenuEntry(
				"APISettings",
				LOCTEXT("APISettings", "API Settings"),
				LOCTEXT("APISettingsTip", "Open Project Settings for Access Key / Secret Key"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FSumeruAIATFEditorModule::OpenApiSettings)));
			AtfSection.AddMenuEntry(
				"CreateDemo",
				LOCTEXT("CreateDemo", "Create ATF Demo Level"),
				LOCTEXT("CreateDemoTip", "Create or open Content/SumeruAI/Sample/AudioToFace/ATF_Demo"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FSumeruAIATFEditorModule::CreateAtfDemoLevel)));
			AtfSection.AddMenuEntry(
				"SetupATF",
				LOCTEXT("SetupATF", "Setup ATF on Selected Actor"),
				LOCTEXT("SetupATFTip", "Add UATFComponent to the selected MetaHuman / ARKit character. Does not replace the Face AnimBP."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FSumeruAIATFEditorModule::SetupAtfOnSelection)));
		}));

	UToolMenu* ActorMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.ActorContextMenu");
	FToolMenuSection& ActorSection = ActorMenu->FindOrAddSection("SumeruAI");
	ActorSection.AddMenuEntry(
		"SetupATFActor",
		LOCTEXT("SetupATFActor", "Setup SumeruAI ATF"),
		LOCTEXT("SetupATFActorTip", "Wire Audio-to-Face onto this character (keeps the existing Face AnimBP)."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateStatic(&FSumeruAIATFEditorModule::SetupAtfOnSelection)));

	UToolMenu* BlueprintMenu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu.Blueprint");
	FToolMenuSection& BlueprintSection = BlueprintMenu->FindOrAddSection("SumeruAI");
	BlueprintSection.AddDynamicEntry(
		"SetupATFBlueprint",
		FNewToolMenuSectionDelegate::CreateStatic(&FSumeruAIATFEditorModule::PopulateBlueprintAssetMenu));
}

void FSumeruAIATFEditorModule::OpenApiSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->ShowViewer(TEXT("Project"), TEXT("SumeruAI"), TEXT("ATF API"));
	}
}

void FSumeruAIATFEditorModule::CreateAtfDemoLevel()
{
	const FString PackageName = TEXT("/SumeruAI/SumeruAI/Sample/AudioToFace/ATF_Demo");
	const FString Filename = FPackageName::LongPackageNameToFilename(
		PackageName, FPackageName::GetMapPackageExtension());
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);

	UWorld* World = nullptr;
	const bool bAlreadyExists = FPaths::FileExists(Filename);
	if (bAlreadyExists)
	{
		World = UEditorLoadingAndSavingUtils::LoadMap(Filename);
	}
	else
	{
		World = UEditorLoadingAndSavingUtils::NewMapFromTemplate(
			TEXT("/Engine/Maps/Templates/Template_Default"), false);
		if (!World)
		{
			World = UEditorLoadingAndSavingUtils::NewBlankMap(false);
		}
	}

	if (!World)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoWorld", "Could not create a new map."));
		return;
	}

	const bool bChanged = EnsureBasicLevelEnvironment(World) | EnsureDemoActors(World);
	if (!bAlreadyExists || bChanged)
	{
		if (!UEditorLoadingAndSavingUtils::SaveMap(World, PackageName))
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveFailed", "Updated the demo actors but failed to save ATF_Demo."));
			return;
		}
	}

	FMessageDialog::Open(
		EAppMsgType::Ok,
		bAlreadyExists
			? LOCTEXT("DemoLoaded",
				"Opened ATF_Demo (Basic lighting).\n1. Edit -> Project Settings -> SumeruAI -> ATF API\n2. Import your MetaHuman (or any ARKit face) into the level\n3. Select it -> SumeruAI -> Setup ATF on Selected Actor\n4. Press Play (PIE)")
			: LOCTEXT("DemoCreated",
				"Saved /SumeruAI/SumeruAI/Sample/AudioToFace/ATF_Demo from the Basic template.\nFill Access Key / Secret Key, import a MetaHuman, then SumeruAI -> Setup ATF on Selected Actor."));
}

void FSumeruAIATFEditorModule::SetupAtfOnSelection()
{
	FSumeruAIATFSetupResult Result = FSumeruAIATFSetup::SetupSelectedActors();
	if (!Result.bSuccess)
	{
		const FSumeruAIATFSetupResult BlueprintResult = FSumeruAIATFSetup::SetupSelectedBlueprints();
		if (BlueprintResult.bSuccess)
		{
			Result = BlueprintResult;
		}
		else if (!BlueprintResult.Message.IsEmpty()
			&& !BlueprintResult.Message.StartsWith(TEXT("Select an Actor Blueprint")))
		{
			Result.Message += TEXT("\n") + BlueprintResult.Message;
		}
	}

	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Result.Message));
}

void FSumeruAIATFEditorModule::PopulateBlueprintAssetMenu(FToolMenuSection& InSection)
{
	const UContentBrowserAssetContextMenuContext* Context = InSection.FindContext<UContentBrowserAssetContextMenuContext>();
	if (!Context)
	{
		return;
	}

	bool bHasActorBlueprint = false;
	for (const FAssetData& Asset : Context->SelectedAssets)
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>(Asset.GetAsset()))
		{
			if (Blueprint->ParentClass && Blueprint->ParentClass->IsChildOf(AActor::StaticClass()))
			{
				bHasActorBlueprint = true;
				break;
			}
		}
	}
	if (!bHasActorBlueprint)
	{
		return;
	}

	InSection.AddMenuEntry(
		"SetupATFBlueprintEntry",
		LOCTEXT("SetupATFBlueprint", "Setup SumeruAI ATF"),
		LOCTEXT("SetupATFBlueprintTip", "Add UATFComponent. Face mesh is found at Play; Face AnimBP is not replaced."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			const FSumeruAIATFSetupResult Result = FSumeruAIATFSetup::SetupSelectedBlueprints();
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Result.Message));
		})));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSumeruAIATFEditorModule, SumeruAIATFEditor)
