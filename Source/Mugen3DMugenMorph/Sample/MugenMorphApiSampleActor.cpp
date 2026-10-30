// Copyright 2026 Mugen3D. All Rights Reserved.

#include "MugenMorphApiSampleActor.h"

#include "MugenMorphApiClient.h"
#include "MugenMorphApiSettings.h"
#include "MugenMorphAudioRecord.h"
#include "MugenMorphComponent.h"
#include "MugenMorph_Type.h"
#include "Input/Reply.h"
#include "MugenMorphLog.h"
#include "MugenMorphProjectSettings.h"
#include "MugenMorphSubsystem.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

#if WITH_EDITOR
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#endif

namespace MugenMorphSampleOverlayPrivate
{
	static TMap<uint32, TSharedPtr<SWidget>> Widgets;
}

AMugenMorphApiSampleActor::AMugenMorphApiSampleActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Client = CreateDefaultSubobject<UMugenMorphApiClient>(TEXT("MugenMorphApiClient"));
	Recorder = new FMugenMorphAudioRecord();
	StatusMessage = TEXT("Mugen3D MugenMorph");
}

AMugenMorphApiSampleActor::~AMugenMorphApiSampleActor()
{
	delete Recorder;
	Recorder = nullptr;
}

void AMugenMorphApiSampleActor::BeginPlay()
{
	Super::BeginPlay();

	EnsureRuntime();
	BindClientEvents();
	BindInput();

	if (bShowOverlay)
	{
		ShowOverlay();
	}

	if (!HasCredentials())
	{
		SetStatus(TEXT("No Access Key. Edit -> Project Settings -> Mugen3D -> MugenMorph API"));
		UE_LOG(LogMugenMorph, Warning, TEXT("%s"), *StatusMessage);
	}
	else if (bLoginOnBeginPlay && Client)
	{
		SetStatus(TEXT("Logging in..."));
		Client->Login();
	}
}

void AMugenMorphApiSampleActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideOverlay();
	if (Recorder)
	{
		Recorder->Abort();
	}
	Super::EndPlay(EndPlayReason);
}

void AMugenMorphApiSampleActor::EnsureRuntime()
{
	if (UMugenMorphSubsystem* Subsystem = UMugenMorphSubsystem::Get(this))
	{
		Subsystem->EnsureManager();
	}

	if (!FaceMesh)
	{
		FaceMesh = UMugenMorphComponent::FindFaceSkeletalMeshInWorld(this, this);
	}

	if (UMugenMorphComponent* Face = EnsureFaceComponent())
	{
		if (Client)
		{
			Client->FaceId = FaceId;
		}
		Face->bReplaceAnimInstance = false;
		Face->Init(FaceId, MotionType::ARKit, FaceMesh, nullptr, false);
	}
	else
	{
		SetStatus(TEXT("No face mesh. Import a MetaHuman, select it, then Mugen3D -> Setup MugenMorph on Selected Actor."));
	}
}

UMugenMorphComponent* AMugenMorphApiSampleActor::EnsureFaceComponent() const
{
	if (!FaceMesh)
	{
		return nullptr;
	}

	AActor* OwnerActor = FaceMesh->GetOwner();
	if (!OwnerActor)
	{
		return nullptr;
	}

	UMugenMorphComponent* Face = OwnerActor->FindComponentByClass<UMugenMorphComponent>();
	if (!Face)
	{
		Face = NewObject<UMugenMorphComponent>(OwnerActor, TEXT("MugenMorphComponent"));
		OwnerActor->AddInstanceComponent(Face);
		Face->RegisterComponent();
	}
	return Face;
}

void AMugenMorphApiSampleActor::BindClientEvents()
{
	if (!Client)
	{
		return;
	}
	Client->OnLoginSucceeded.AddDynamic(this, &AMugenMorphApiSampleActor::HandleLoginSucceeded);
	Client->OnRequestSucceeded.AddDynamic(this, &AMugenMorphApiSampleActor::HandleRequestSucceeded);
	Client->OnRequestFailed.AddDynamic(this, &AMugenMorphApiSampleActor::HandleRequestFailed);
}

void AMugenMorphApiSampleActor::BindInput()
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}
	EnableInput(PC);
	if (!InputComponent)
	{
		return;
	}
	InputComponent->BindKey(EKeys::J, IE_Pressed, this, &AMugenMorphApiSampleActor::InputBrowseWav);
	InputComponent->BindKey(EKeys::I, IE_Pressed, this, &AMugenMorphApiSampleActor::InputInterrupt);
	InputComponent->BindKey(EKeys::R, IE_Pressed, this, &AMugenMorphApiSampleActor::InputToggleRecord);
}

void AMugenMorphApiSampleActor::InputBrowseWav()
{
	BrowseAndPlayWav();
}

void AMugenMorphApiSampleActor::InputInterrupt()
{
	InterruptPlayback();
}

void AMugenMorphApiSampleActor::InputToggleRecord()
{
	if (IsRecording())
	{
		StopRecord();
	}
	else
	{
		StartRecord();
	}
}

bool AMugenMorphApiSampleActor::HasCredentials() const
{
	if (!Client)
	{
		return false;
	}
	if (!Client->AccessKeyOverride.IsEmpty() && !Client->SecretKeyOverride.IsEmpty())
	{
		return true;
	}
	if (Client->Settings && Client->Settings->HasCredentials())
	{
		return true;
	}
	if (const UMugenMorphProjectSettings* ProjectSettings = GetDefault<UMugenMorphProjectSettings>())
	{
		return !ProjectSettings->AccessKey.IsEmpty() && !ProjectSettings->SecretKey.IsEmpty();
	}
	return false;
}

void AMugenMorphApiSampleActor::SetStatus(const FString& Message)
{
	StatusMessage = Message;
	UE_LOG(LogMugenMorph, Log, TEXT("MugenMorph sample: %s"), *StatusMessage);
}

void AMugenMorphApiSampleActor::ShowOverlay()
{
	UGameViewportClient* Viewport = GetWorld() ? GetWorld()->GetGameViewport() : nullptr;
	if (!Viewport)
	{
		return;
	}

	HideOverlay();

	TSharedRef<SWidget> Overlay = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(16.f)
		[
			SNew(SBox)
			.WidthOverride(420.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.72f))
				.Padding(12.f)
				[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Mugen3D Audio-to-Face")))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text_Lambda([this]() { return FText::FromString(StatusMessage); })
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("J Browse   R Record   I Interrupt")))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("Browse...")))
						.OnClicked_Lambda([this]()
						{
							BrowseAndPlayWav();
							return FReply::Handled();
						})
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("Interrupt")))
						.OnClicked_Lambda([this]()
						{
							InterruptPlayback();
							return FReply::Handled();
						})
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
					[
						SNew(SButton)
						.Text_Lambda([this]()
						{
							return FText::FromString(IsRecording() ? TEXT("Recording...") : TEXT("Start Record"));
						})
						.OnClicked_Lambda([this]()
						{
							StartRecord();
							return FReply::Handled();
						})
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("Stop Record")))
						.OnClicked_Lambda([this]()
						{
							StopRecord();
							return FReply::Handled();
						})
					]
				]
			]
		]
	];

	Viewport->AddViewportWidgetContent(Overlay, 50);
	MugenMorphSampleOverlayPrivate::Widgets.Add(GetUniqueID(), Overlay);
}

void AMugenMorphApiSampleActor::HideOverlay()
{
	TSharedPtr<SWidget> Overlay;
	if (MugenMorphSampleOverlayPrivate::Widgets.RemoveAndCopyValue(GetUniqueID(), Overlay) && Overlay.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			if (UGameViewportClient* Viewport = World->GetGameViewport())
			{
				Viewport->RemoveViewportWidgetContent(Overlay.ToSharedRef());
			}
		}
	}
}

bool AMugenMorphApiSampleActor::RequestFromWavFile()
{
	return RequestFromWavFilePath(WavFilePath);
}

bool AMugenMorphApiSampleActor::RequestFromWavFilePath(const FString& AbsoluteOrRelativePath)
{
	if (!Client)
	{
		SetStatus(TEXT("Client component missing."));
		return false;
	}
	if (AbsoluteOrRelativePath.IsEmpty())
	{
		SetStatus(TEXT("WavFilePath is empty. Press J to browse."));
		return false;
	}
	WavFilePath = AbsoluteOrRelativePath;
	SetStatus(FString::Printf(TEXT("Requesting MugenMorph: %s"), *WavFilePath));
	return Client->RequestFromWavFile(WavFilePath, TEXT("start"));
}

bool AMugenMorphApiSampleActor::BrowseAndPlayWav()
{
#if WITH_EDITOR
	IDesktopPlatform* Desktop = FDesktopPlatformModule::Get();
	if (!Desktop)
	{
		SetStatus(TEXT("File dialog unavailable."));
		return false;
	}
	TArray<FString> Files;
	const void* Parent = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	const bool bOpened = Desktop->OpenFileDialog(
		Parent,
		TEXT("Select 16-bit PCM WAV"),
		FPaths::ProjectDir(),
		TEXT(""),
		TEXT("WAV files (*.wav)|*.wav"),
		EFileDialogFlags::None,
		Files);
	if (!bOpened || Files.Num() == 0)
	{
		return false;
	}
	return RequestFromWavFilePath(Files[0]);
#else
	SetStatus(TEXT("Browse is Editor/PIE only."));
	return false;
#endif
}

void AMugenMorphApiSampleActor::InterruptPlayback()
{
	if (Client)
	{
		Client->Interrupt();
	}
	SetStatus(TEXT("Interrupted."));
}

void AMugenMorphApiSampleActor::StartRecord()
{
	if (!Recorder)
	{
		Recorder = new FMugenMorphAudioRecord();
	}
	if (Recorder->IsRecording())
	{
		return;
	}

	FModuleManager::Get().LoadModule(TEXT("AudioCapture"));

	if (!Recorder->Start(MaxRecordSeconds))
	{
		SetStatus(TEXT("Record failed. Check the default microphone."));
		return;
	}
	SetStatus(TEXT("Recording... press Stop Record or R."));
}

void AMugenMorphApiSampleActor::StopRecord()
{
	if (!Recorder || !Recorder->IsRecording())
	{
		return;
	}

	TArray<uint8> WavBytes;
	if (!Recorder->Stop(WavBytes))
	{
		SetStatus(TEXT("Record failed (empty capture)."));
		return;
	}
	if (!Client)
	{
		SetStatus(TEXT("Client component missing."));
		return;
	}

	Client->Interrupt();
	SetStatus(FString::Printf(TEXT("Recorded %d bytes. Sending MugenMorph..."), WavBytes.Num()));
	Client->PlayFromAudio(WavBytes);
}

bool AMugenMorphApiSampleActor::IsRecording() const
{
	return Recorder && Recorder->IsRecording();
}

void AMugenMorphApiSampleActor::HandleLoginSucceeded(const FString& AccessToken)
{
	SetStatus(TEXT("Login ok. Press J to browse a WAV, or R to record."));
}

void AMugenMorphApiSampleActor::HandleRequestSucceeded(const FMugenMorphRepData& Response, const FString& TraceId)
{
	SetStatus(FString::Printf(TEXT("MugenMorph ok (trace %s). Playing."), *TraceId));
}

void AMugenMorphApiSampleActor::HandleRequestFailed(const FString& ErrorMessage)
{
	SetStatus(ErrorMessage.IsEmpty() ? TEXT("MugenMorph request failed.") : ErrorMessage);
}
