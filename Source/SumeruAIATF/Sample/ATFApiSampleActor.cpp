// Copyright SumeruAI. All Rights Reserved.

#include "ATFApiSampleActor.h"

#include "ATFApiClient.h"
#include "ATFApiSettings.h"
#include "ATFComponent.h"
#include "ATF_Type.h"
#include "Input/Reply.h"
#include "SumeruATFLog.h"
#include "SumeruATFProjectSettings.h"
#include "SumeruATFSubsystem.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Misc/Paths.h"
#include "Widgets/SWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#if WITH_EDITOR
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#endif

namespace ATFSampleOverlayPrivate
{
	static TMap<uint32, TSharedPtr<SWidget>> Widgets;
}

AATFApiSampleActor::AATFApiSampleActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Client = CreateDefaultSubobject<UATFApiClient>(TEXT("ATFApiClient"));
	StatusMessage = TEXT("SumeruAI ATF");
}

void AATFApiSampleActor::BeginPlay()
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
		SetStatus(TEXT("No Access Key. Edit -> Project Settings -> SumeruAI -> ATF API"));
		UE_LOG(LogSumeruATF, Warning, TEXT("%s"), *StatusMessage);
	}
	else if (bLoginOnBeginPlay && Client)
	{
		SetStatus(TEXT("Logging in..."));
		Client->Login();
	}

	if (bRequestOnBeginPlay)
	{
		PlayFromAudioFile();
	}
}

void AATFApiSampleActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideOverlay();
	Super::EndPlay(EndPlayReason);
}

void AATFApiSampleActor::EnsureRuntime()
{
	if (USumeruATFSubsystem* Subsystem = USumeruATFSubsystem::Get(this))
	{
		Subsystem->EnsureManager();
	}

	if (!FaceMesh)
	{
		FaceMesh = UATFComponent::FindFaceSkeletalMeshInWorld(this, this);
	}

	if (UATFComponent* Face = EnsureFaceComponent())
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
		SetStatus(TEXT("No face mesh. Import a MetaHuman, select it, then SumeruAI -> Setup ATF on Selected Actor."));
	}
}

UATFComponent* AATFApiSampleActor::EnsureFaceComponent() const
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

	UATFComponent* Face = OwnerActor->FindComponentByClass<UATFComponent>();
	if (!Face)
	{
		Face = NewObject<UATFComponent>(OwnerActor, TEXT("ATFComponent"));
		OwnerActor->AddInstanceComponent(Face);
		Face->RegisterComponent();
	}
	return Face;
}

void AATFApiSampleActor::BindClientEvents()
{
	if (!Client)
	{
		return;
	}
	Client->OnLoginSucceeded.AddDynamic(this, &AATFApiSampleActor::HandleLoginSucceeded);
	Client->OnRequestSucceeded.AddDynamic(this, &AATFApiSampleActor::HandleRequestSucceeded);
	Client->OnRequestFailed.AddDynamic(this, &AATFApiSampleActor::HandleRequestFailed);
}

void AATFApiSampleActor::BindInput()
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
	InputComponent->BindKey(EKeys::J, IE_Pressed, this, &AATFApiSampleActor::InputBrowseWav);
	InputComponent->BindKey(EKeys::P, IE_Pressed, this, &AATFApiSampleActor::InputPlayWav);
	InputComponent->BindKey(EKeys::I, IE_Pressed, this, &AATFApiSampleActor::InputInterrupt);
}

void AATFApiSampleActor::InputPlayWav()
{
	PlayFromAudioFile();
}

void AATFApiSampleActor::InputBrowseWav()
{
	BrowseAndPlayWav();
}

void AATFApiSampleActor::InputInterrupt()
{
	InterruptPlayback();
}

bool AATFApiSampleActor::HasCredentials() const
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
	if (const USumeruATFProjectSettings* ProjectSettings = GetDefault<USumeruATFProjectSettings>())
	{
		return !ProjectSettings->AccessKey.IsEmpty() && !ProjectSettings->SecretKey.IsEmpty();
	}
	return false;
}

void AATFApiSampleActor::SetStatus(const FString& Message)
{
	StatusMessage = Message;
	UE_LOG(LogSumeruATF, Log, TEXT("ATF sample: %s"), *StatusMessage);
}

void AATFApiSampleActor::ShowOverlay()
{
	UGameViewportClient* Viewport = GetWorld() ? GetWorld()->GetGameViewport() : nullptr;
	if (!Viewport)
	{
		return;
	}

	HideOverlay();

	TSharedRef<SWidget> Overlay = SNew(SBox)
		.WidthOverride(460.f)
		.Padding(FMargin(16.f))
		[
			SNew(SBorder)
			.Padding(12.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("SumeruAI Audio-to-Face")))
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
					.Text(FText::FromString(TEXT("P Play WAV   J Browse   I Interrupt")))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("Play WAV")))
						.OnClicked_Lambda([this]()
						{
							PlayFromAudioFile();
							return FReply::Handled();
						})
					]
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
			]
		];

	Viewport->AddViewportWidgetContent(Overlay, 50);
	ATFSampleOverlayPrivate::Widgets.Add(GetUniqueID(), Overlay);
}

void AATFApiSampleActor::HideOverlay()
{
	TSharedPtr<SWidget> Overlay;
	if (ATFSampleOverlayPrivate::Widgets.RemoveAndCopyValue(GetUniqueID(), Overlay) && Overlay.IsValid())
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

bool AATFApiSampleActor::RequestFromWavFile()
{
	return RequestFromWavFilePath(WavFilePath);
}

bool AATFApiSampleActor::RequestFromWavFilePath(const FString& AbsoluteOrRelativePath)
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
	SetStatus(FString::Printf(TEXT("Requesting ATF: %s"), *WavFilePath));
	return Client->RequestFromWavFile(WavFilePath, TEXT("start"));
}

bool AATFApiSampleActor::PlayFromAudioFile()
{
	return RequestFromWavFilePath(WavFilePath);
}

bool AATFApiSampleActor::BrowseAndPlayWav()
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
	SetStatus(TEXT("Browse is Editor/PIE only. Set WavFilePath on the actor."));
	return PlayFromAudioFile();
#endif
}

void AATFApiSampleActor::InterruptPlayback()
{
	if (Client)
	{
		Client->Interrupt();
	}
	SetStatus(TEXT("Interrupted."));
}

void AATFApiSampleActor::HandleLoginSucceeded(const FString& AccessToken)
{
	SetStatus(TEXT("Login ok. Press P to play WAV, or J to browse."));
}

void AATFApiSampleActor::HandleRequestSucceeded(const FATFRepData& Response, const FString& TraceId)
{
	SetStatus(FString::Printf(TEXT("ATF ok (trace %s). Playing."), *TraceId));
}

void AATFApiSampleActor::HandleRequestFailed(const FString& ErrorMessage)
{
	SetStatus(ErrorMessage.IsEmpty() ? TEXT("ATF request failed.") : ErrorMessage);
}
