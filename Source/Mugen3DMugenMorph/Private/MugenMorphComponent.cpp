// Copyright 2026 Mugen3D. All Rights Reserved.

#include "MugenMorphComponent.h"
#include "MugenMorphApiSampleActor.h"
#include "MugenMorph.h"
#include "SumeruMugenMorphLog.h"
#include "SumeruMugenMorphSubsystem.h"
#include "Mugen3DLog.h"
#include "Animation/AnimInstance.h"
#include "Animation/MorphTarget.h"
#include "Misc/ScopeLock.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Math/Vector.h"
#include "Math/Quat.h"

const TArray<FName>& UMugenMorphComponent::GetARKitNames()
{
	static const TArray<FName> Names = {
		FName(TEXT("EyeBlinkLeft")),
		FName(TEXT("EyeLookDownLeft")),
		FName(TEXT("EyeLookInLeft")),
		FName(TEXT("EyeLookOutLeft")),
		FName(TEXT("EyeLookUpleft")),
		FName(TEXT("EyeSquintLeft")),
		FName(TEXT("EyeWideLeft")),
		FName(TEXT("EyeBlinkRight")),
		FName(TEXT("EyeLookDownRight")),
		FName(TEXT("EyeLookInRight")),
		FName(TEXT("EyeLookOutRight")),
		FName(TEXT("EyeLookUpRight")),
		FName(TEXT("EyeSquintRight")),
		FName(TEXT("EyeWideRight")),
		FName(TEXT("JawForward")),
		FName(TEXT("JawLeft")),
		FName(TEXT("JawRight")),
		FName(TEXT("JawOpen")),
		FName(TEXT("MouthClose")),
		FName(TEXT("MouthFunnel")),
		FName(TEXT("MouthPucker")),
		FName(TEXT("MouthLeft")),
		FName(TEXT("MouthRight")),
		FName(TEXT("MouthSmileLeft")),
		FName(TEXT("MouthSmileRight")),
		FName(TEXT("MouthFrownLeft")),
		FName(TEXT("MouthFrownRight")),
		FName(TEXT("MouthDimpleLeft")),
		FName(TEXT("MouthDimpleRight")),
		FName(TEXT("MouthStretchLeft")),
		FName(TEXT("MouthStretchRight")),
		FName(TEXT("MouthRollLower")),
		FName(TEXT("MouthRollUpper")),
		FName(TEXT("MouthShrugLower")),
		FName(TEXT("MouthShrugUpper")),
		FName(TEXT("MouthPressLeft")),
		FName(TEXT("MouthPressRight")),
		FName(TEXT("MouthLowerDownLeft")),
		FName(TEXT("MouthLowerDownRight")),
		FName(TEXT("MouthUpperUpLeft")),
		FName(TEXT("MouthUpperUpRight")),
		FName(TEXT("BrowDownLeft")),
		FName(TEXT("BrowDownRight")),
		FName(TEXT("BrowInnerUp")),
		FName(TEXT("BrowOuterUpLeft")),
		FName(TEXT("BrowOuterUpRight")),
		FName(TEXT("CheekPuff")),
		FName(TEXT("CheekSquintLeft")),
		FName(TEXT("CheekSquintRight")),
		FName(TEXT("NoseSneerLeft")),
		FName(TEXT("NoseSneerRight")),
		FName(TEXT("TongueOut")),
		FName(TEXT("HeadYaw")),
		FName(TEXT("HeadPitch")),
		FName(TEXT("HeadRoll")),
		FName(TEXT("LeftEyeYaw")),
		FName(TEXT("LeftEyePitch")),
		FName(TEXT("LeftEyeRoll")),
		FName(TEXT("RightEyeYaw")),
		FName(TEXT("RightEyePitch")),
		FName(TEXT("RightEyeRoll")),
	};
	return Names;
}

FName UMugenMorphComponent::ARKitEnumToString(int32 Index)
{
	const TArray<FName>& Names = GetARKitNames();
	if (Names.IsValidIndex(Index))
	{
		return Names[Index];
	}
	return FName(TEXT("UNKNOWN"));
}

AMugenMorphManager* UMugenMorphComponent::ResolveManager() const
{
	if (USumeruMugenMorphSubsystem* Subsystem = USumeruMugenMorphSubsystem::Get(this))
	{
		if (AMugenMorphManager* Manager = Subsystem->EnsureManager())
		{
			return Manager;
		}
	}

	if (AMugenMorphManager* Manager = AMugenMorphManager::Instance())
	{
		return Manager;
	}

	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AMugenMorphManager> It(World); It; ++It)
		{
			return *It;
		}
	}

	return nullptr;
}

void UMugenMorphComponent::UpdateTickPolicy()
{
	SetComponentTickEnabled(bIsPlayingEmote || bEnableIdleEmote);
}

void UMugenMorphComponent::BindFaceMeshTick()
{
	if (!FaceCom)
	{
		return;
	}
	PrimaryComponentTick.AddPrerequisite(FaceCom, FaceCom->PrimaryComponentTick);
}

void UMugenMorphComponent::AddMorphTargetAlias(FName ARKitName, FName MorphName)
{
	if (ARKitName.IsNone() || MorphName.IsNone())
	{
		return;
	}
	TArray<FName>& Targets = ARKitMappingDic.FindOrAdd(ARKitName);
	Targets.AddUnique(MorphName);
}

void UMugenMorphComponent::ApplyMorphValue(FName ARKitName, float Value)
{
	if (!FaceCom)
	{
		return;
	}

	TArray<FName> Names;
	if (const TArray<FName>* Mapped = ARKitMappingDic.Find(ARKitName))
	{
		Names.Append(*Mapped);
	}
	Names.AddUnique(ARKitName);
	GetMetaHumanCtrlAliases(ARKitName, Names);

	for (const FName& Name : Names)
	{
		FaceCom->SetMorphTarget(Name, Value);
	}
}

void UMugenMorphComponent::BuildFlameNameCache()
{
	FlamePositiveNames.Reset();
	FlameNegativeNames.Reset();
	FlamePositiveNames.Reserve(50);
	FlameNegativeNames.Reserve(50);
	for (int32 i = 0; i < 50; ++i)
	{
		const FString Base = FString::Printf(TEXT("Expression_%d"), i + 300);
		FlamePositiveNames.Add(FName(*FString::Printf(TEXT("P%s"), *Base)));
		FlameNegativeNames.Add(FName(*FString::Printf(TEXT("N%s"), *Base)));
	}
}

void UMugenMorphComponent::ChangeState(EVHState State)
{
	AvatarState = State;
}

void UMugenMorphComponent::HandleIdle(float DeltaTime)
{
	if (AvatarState != EVHState::Idle)
	{
		return;
	}

	TempTime += DeltaTime;
	if (TempTime >= IdleIntervalSeconds)
	{
		TempTime = 0.f;
		ChangeState(EVHState::Motion);
		RandomIdleHandle(DeltaTime);
	}
}

void UMugenMorphComponent::OnStartSpeech()
{
	ChangeState(EVHState::Speech);
}

void UMugenMorphComponent::OnStopSpeech()
{
	TempTime = 0.f;
	ChangeState(EVHState::Idle);
}

void UMugenMorphComponent::OnStopMotion()
{
	TempTime = 0.f;
	ChangeState(EVHState::Idle);
}

UMugenMorphComponent::UMugenMorphComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	// After Face AnimBP / RigLogic so SetMorphTarget is not wiped the same frame.
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UMugenMorphComponent::BeginPlay()
{
	Super::BeginPlay();
	if (bAutoInitOnBeginPlay)
	{
		if (!FaceCom)
		{
			FaceCom = FindFaceSkeletalMesh(GetOwner());
		}
		if (FaceCom)
		{
			Init(Id, CurMotionType, FaceCom);
		}
		else
		{
			UE_LOG(LogSumeruMugenMorph, Warning,
				TEXT("UMugenMorphComponent::BeginPlay: no face mesh on '%s'. Assign FaceCom or use Mugen3D -> Setup MugenMorph."),
				*GetNameSafe(GetOwner()));
		}
	}
	UpdateTickPolicy();
}

void UMugenMorphComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AMugenMorphManager* Manager = ResolveManager())
	{
		Manager->UnRegisterFace(Id);
	}
	Super::EndPlay(EndPlayReason);
}

void UMugenMorphComponent::SetPlayingEmote(bool bPlaying)
{
	bIsPlayingEmote = bPlaying;
	UpdateTickPolicy();
}

void UMugenMorphComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsPlayingEmote)
	{
		PlayEmote(DeltaTime);
	}
	else if (bEnableIdleEmote)
	{
		HandleIdle(DeltaTime);
	}
}

void UMugenMorphComponent::StartEmote(FEmoteData& EmoteData)
{
	StartEmote(MoveTemp(EmoteData));
}

void UMugenMorphComponent::StitchChatEmoteBoundary(FEmoteData& EmoteData) const
{
	if (TempBs.Num() <= 0 || EmoteData.EmoteValues.Num() <= 0)
	{
		return;
	}

	const int32 BlendFrames = FMath::Clamp(EmoteData.EmoteValues.Num(), 1, 8);
	for (int32 i = 0; i < BlendFrames; ++i)
	{
		TArray<float>& Dst = EmoteData.EmoteValues[i].Data;
		if (Dst.Num() <= 0)
		{
			continue;
		}
		const float T = static_cast<float>(i + 1) / static_cast<float>(BlendFrames);
		const int32 Count = FMath::Min(TempBs.Num(), Dst.Num());
		for (int32 C = 0; C < Count; ++C)
		{
			Dst[C] = TempBs[C] * (1.f - T) + Dst[C] * T;
		}
	}
}

void UMugenMorphComponent::StartEmote(FEmoteData&& EmoteData)
{
	// Match Mugen3D-Unity Chat append. Do NOT force dual-queue crossfade on every chunk:
	// GetData clears the inactive queue when Weight hits 0 and would wipe unplayed frames,
	// leaving the mouth frozen while audio continues.
	const float IncomingInterval = EmoteData.Interval > KINDA_SMALL_NUMBER ? EmoteData.Interval : (1.f / 30.f);
	const EMugenMorphDataType IncomingType = EmoteData.aTFDataType;
	const int32 IncomingFrames = EmoteData.EmoteValues.Num();

	if (IncomingType != EMugenMorphDataType::Chat || ChatBox == -1 || CurInterval <= KINDA_SMALL_NUMBER)
	{
		CurInterval = IncomingInterval;
	}

	if (IncomingType == EMugenMorphDataType::Motion)
	{
		if (BSData.IsEmpty() && BSDataBlender.IsEmpty())
		{
			if (EmoteData.EmoteValues.Num() > 0)
			{
				TempBs = EmoteData.EmoteValues[0].Data;
			}
			SetBSData(MoveTemp(EmoteData));
		}
		else if (!BSData.IsEmpty() && BSDataBlender.IsEmpty())
		{
			Dir = 1;
			Weight = 1.f;
			SetBSDataBlender(MoveTemp(EmoteData));
		}
		else if (BSData.IsEmpty() && !BSDataBlender.IsEmpty())
		{
			Dir = -1;
			Weight = 1.f;
			SetBSData(MoveTemp(EmoteData));
		}
	}
	else if (IncomingType == EMugenMorphDataType::Chat)
	{
		OnStartSpeech();

		const bool bContinuing =
			CurType == EMugenMorphDataType::Chat
			&& (bIsPlayingEmote || !BSData.IsEmpty() || !BSDataBlender.IsEmpty() || ChatBox != -1);
		if (bContinuing && EmoteData.EmoteValues.Num() > 0)
		{
			StitchChatEmoteBoundary(EmoteData);
		}

		if (ChatBox == -1)
		{
			if (BSData.IsEmpty() && BSDataBlender.IsEmpty())
			{
				if (EmoteData.EmoteValues.Num() > 0)
				{
					TempBs = EmoteData.EmoteValues[0].Data;
				}
				SetBSData(MoveTemp(EmoteData));
				ChatBox = 0;
			}
			else if (BSData.IsEmpty() && !BSDataBlender.IsEmpty())
			{
				SetBSData(MoveTemp(EmoteData));
				ChatBox = 0;
				Dir = -1;
				Weight = 1.f;
			}
			else if (!BSData.IsEmpty() && BSDataBlender.IsEmpty())
			{
				SetBSDataBlender(MoveTemp(EmoteData));
				ChatBox = 1;
				Dir = 1;
				Weight = 1.f;
			}
		}
		else if (ChatBox == 0)
		{
			SetBSData(MoveTemp(EmoteData));
		}
		else if (ChatBox == 1)
		{
			SetBSDataBlender(MoveTemp(EmoteData));
		}
	}

	CurType = IncomingType;
	if (IncomingType == EMugenMorphDataType::Chat)
	{
		ChatSessionFrameCount += IncomingFrames;
		bChatClosingDecay = false;
	}
	else
	{
		PlayedFrameIndex = 0;
		ChatSessionFrameCount = IncomingFrames;
		ChatEmotePlayedSeconds = 0.f;
		CacheTime = 0.f;
	}
	SetPlayingEmote(true);
}

void UMugenMorphComponent::PlayEmote(float DeltaTime)
{
	// Match Mugen3D-Unity timing (fps interval). Hold last pose across segment gaps while audio lives.
	if (!BSData.IsEmpty() || !BSDataBlender.IsEmpty())
	{
		const float Step = CurInterval > KINDA_SMALL_NUMBER ? CurInterval : (1.f / 30.f);

		SetBlendShapeValue(TempBs);

		CacheTime += DeltaTime;
		int32 Safety = 0;
		while (CacheTime >= Step && Step > 0.f && Safety < 8
			&& (!BSData.IsEmpty() || !BSDataBlender.IsEmpty()))
		{
			CacheTime -= Step;
			GetData(DeltaTime, TempBs);
			ChatEmotePlayedSeconds += Step;
			++PlayedFrameIndex;
			++Safety;
		}
	}
	else
	{
		// Segment gap: keep ChatBox / last morph so the next chunk can append or crossfade.
		if (CurType == EMugenMorphDataType::Chat)
		{
			if (AMugenMorphManager* Manager = ResolveManager())
			{
				if (Manager->IsProceduralSpeechActive())
				{
					SetBlendShapeValue(TempBs);
					return;
				}
			}
		}

		SetPlayingEmote(false);
		CacheTime = 0.f;
		Weight = 1.f;
		ChatBox = -1;
		Dir = 1;
		PlayedFrameIndex = 0;
		ChatSessionFrameCount = 0;
		ChatEmotePlayedSeconds = 0.f;
		bChatClosingDecay = false;
		LastAppliedBs.Reset();

		if (AMugenMorphManager* Manager = ResolveManager())
		{
			if (CurType == EMugenMorphDataType::Chat)
			{
				Manager->NotifyStopSpeech();
				OnStopSpeech();
			}
			else if (CurType == EMugenMorphDataType::Motion)
			{
				Manager->NotifyStopMotion();
				OnStopMotion();
			}
		}
		else
		{
			if (CurType == EMugenMorphDataType::Chat)
			{
				OnStopSpeech();
			}
			else if (CurType == EMugenMorphDataType::Motion)
			{
				OnStopMotion();
			}
		}

		CurType = EMugenMorphDataType::None;
		IsBusy = false;
	}
}

void UMugenMorphComponent::SetBSData(FEmoteData&& EmoteData)
{
	for (FAudioExpression& Expr : EmoteData.EmoteValues)
	{
		BSData.Append(MoveTemp(Expr));
	}
}

void UMugenMorphComponent::SetBSDataBlender(FEmoteData&& EmoteData)
{
	for (FAudioExpression& Expr : EmoteData.EmoteValues)
	{
		BSDataBlender.Append(MoveTemp(Expr));
	}
}

bool UMugenMorphComponent::NearlyEqualBs(const TArray<float>& A, const TArray<float>& B) const
{
	if (A.Num() != B.Num())
	{
		return false;
	}
	for (int32 i = 0; i < A.Num(); ++i)
	{
		if (!FMath::IsNearlyEqual(A[i], B[i], 1e-4f))
		{
			return false;
		}
	}
	return true;
}

void UMugenMorphComponent::SetBlendShapeValue(const TArray<float>& Data)
{
	if (Data.Num() == 0)
	{
		return;
	}

	switch (CurMotionType)
	{
	case MotionType::Bones:
		break;
	case MotionType::ARKit:
		SetARKitBs(Data);
		break;
	case MotionType::FLAME:
		SetFLAMEBS(Data);
		break;
	default:
		break;
	}

	LastAppliedBs = Data;
}

void UMugenMorphComponent::SetARKitBs(const TArray<float>& Data)
{
	if (Data.Num() == 0)
	{
		return;
	}

	{
		FScopeLock Lock(&FaceCurveLock);
		LatestARKitWeights = Data;
	}

	if (!FaceCom)
	{
		return;
	}

	const TArray<FName>& ARKitNames = GetARKitNames();
	const int32 Count = FMath::Min(Data.Num(), ARKitNames.Num());

	for (int32 i = 0; i < Count; ++i)
	{
		float Value = Data[i];
		if (i <= 50)
		{
			Value = FMath::Clamp(Value, 0.f, 1.f);
		}
		ApplyMorphValue(ARKitNames[i], Value);
	}

	if (MugenMorphAnim)
	{
		MugenMorphAnim->FacialTempData.Data = Data;
	}

	if (AMugenMorphManager* Manager = ResolveManager())
	{
		FAudioCacheData& Cache = Manager->FindOrAddAudioCache(Id);
		if (Data.Num() > 54)
		{
			Cache.HeadYaw = Data[52];
			Cache.HeadPitch = Data[53];
			Cache.HeadRoll = Data[54];
		}
	}
}

void UMugenMorphComponent::Interrupt()
{
	BSData.Empty();
	BSDataBlender.Empty();
	ChatBox = -1;
	Dir = 1;
	Weight = 1.f;
	CacheTime = 0.f;

	const int32 DecaySteps = 15;
	int32 Count = 61;
	switch (CurMotionType)
	{
	case MotionType::ARKit:
		Count = 61;
		break;
	case MotionType::FLAME:
		Count = 65;
		break;
	default:
		break;
	}

	if (TempBs.Num() != Count)
	{
		return;
	}

	for (int32 i = 1; i <= DecaySteps; ++i)
	{
		FAudioExpression Fae;
		Fae.Data.Reserve(Count);
		const float Scale = 1.0f - static_cast<float>(i) / DecaySteps;
		for (int32 j = 0; j < Count; ++j)
		{
			Fae.Data.Add(TempBs[j] * Scale);
		}
		BSData.Append(MoveTemp(Fae));
	}

	SetPlayingEmote(true);
}

void UMugenMorphComponent::SetFLAMEBS(const TArray<float>& Data)
{
	if (!FaceCom || Data.Num() < 59)
	{
		return;
	}

	if (FlamePositiveNames.Num() != 50)
	{
		BuildFlameNameCache();
	}

	for (int32 i = 0; i < 50; ++i)
	{
		float Val = Data[i] * 0.1f;
		if (Val > 0.f)
		{
			FaceCom->SetMorphTarget(FlamePositiveNames[i], Val);
			FaceCom->SetMorphTarget(FlameNegativeNames[i], 0.f);
		}
		else
		{
			FaceCom->SetMorphTarget(FlamePositiveNames[i], 0.f);
			FaceCom->SetMorphTarget(FlameNegativeNames[i], -Val);
		}
	}

	const FVector AxisAngle(-Data[56], -Data[58], -Data[57]);
	const FVector Axis = AxisAngle.GetSafeNormal();
	const float Angle = AxisAngle.Size();
	const FQuat Rotation(Axis, Angle);

	if (AMugenMorphManager* Manager = ResolveManager())
	{
		FAudioCacheData& Cache = Manager->FindOrAddAudioCache(Id);
		Cache.JawPitch = Rotation.Rotator().Pitch;
		Cache.JawRoll = Rotation.Rotator().Roll;
		Cache.JawYaw = Rotation.Rotator().Yaw;
	}
}

void UMugenMorphComponent::GetData(float DeltaTime, TArray<float>& OutResult)
{
	FAudioExpression Afe;
	FAudioExpression Afe1;

	if (BSDataBlender.IsEmpty() && !BSData.IsEmpty())
	{
		IsBusy = false;
		BSData.Dequeue(Afe);
		OutResult = MoveTemp(Afe.Data);
		return;
	}

	if (!BSDataBlender.IsEmpty() && !BSData.IsEmpty())
	{
		IsBusy = true;
		// Use one frame-step (not wall DeltaTime): clock catch-up can call GetData
		// multiple times per tick; decaying by DeltaTime each time collapses the blend.
		const float BlendStep = CurInterval > KINDA_SMALL_NUMBER ? CurInterval : DeltaTime;
		Weight -= 5.f * BlendStep;
		Weight = FMath::Clamp(Weight, 0.f, 1.f);
		BSData.Dequeue(Afe);
		BSDataBlender.Dequeue(Afe1);

		const TArray<float>& Bd = Afe.Data;
		const TArray<float>& Bdd = Afe1.Data;
		if (Bd.Num() != Bdd.Num())
		{
			Mugen3D::Mugen3DLog("Blend arrays must have the same length.");
			OutResult = Bd;
			return;
		}

		BlendScratch.SetNumUninitialized(Bd.Num());
		for (int32 i = 0; i < Bd.Num(); ++i)
		{
			if (Dir == 1)
			{
				BlendScratch[i] = Bd[i] * Weight + (1.f - Weight) * Bdd[i];
			}
			else
			{
				BlendScratch[i] = Bd[i] * (1.f - Weight) + Weight * Bdd[i];
			}
		}

		if (Weight == 0.f)
		{
			// Prefer the fade target, but keep any unread frames (Empty would freeze lips mid-speech).
			if (Dir == 1)
			{
				while (!BSData.IsEmpty())
				{
					FAudioExpression Leftover;
					BSData.Dequeue(Leftover);
					BSDataBlender.Append(MoveTemp(Leftover));
				}
				BSData.Empty();
			}
			else
			{
				while (!BSDataBlender.IsEmpty())
				{
					FAudioExpression Leftover;
					BSDataBlender.Dequeue(Leftover);
					BSData.Append(MoveTemp(Leftover));
				}
				BSDataBlender.Empty();
			}
			Dir = 1;
			Weight = 1.f;
		}
		else if (BSData.IsEmpty() || BSDataBlender.IsEmpty())
		{
			Dir = 1;
			Weight = 1.f;
		}

		OutResult = BlendScratch;
		return;
	}

	if (!BSDataBlender.IsEmpty() && BSData.IsEmpty())
	{
		IsBusy = false;
		BSDataBlender.Dequeue(Afe1);
		OutResult = MoveTemp(Afe1.Data);
		return;
	}

	OutResult.Reset();
}

UClass* UMugenMorphComponent::ResolveDefaultAnimClass() const
{
	if (UClass* SoftClass = FaceAnimClass.LoadSynchronous())
	{
		return SoftClass;
	}

	if (AMugenMorphManager* Manager = ResolveManager())
	{
		if (UClass* ManagerClass = Manager->DefaultFaceAnimClass.LoadSynchronous())
		{
			return ManagerClass;
		}
	}

	return UMugenMorphAnim::StaticClass();
}

void UMugenMorphComponent::InitFace(int32 FaceId, MotionType InMotionType, USkeletalMeshComponent* FaceMesh, bool bInReplaceAnimInstance)
{
	Init(FaceId, InMotionType, FaceMesh, ResolveDefaultAnimClass(), bInReplaceAnimInstance);
}

void UMugenMorphComponent::Init(int32 FaceId, MotionType InMotionType, USkeletalMeshComponent* FaceMesh)
{
	Init(FaceId, InMotionType, FaceMesh, ResolveDefaultAnimClass(), bReplaceAnimInstance);
}

void UMugenMorphComponent::Init(int32 FaceId, MotionType InMotionType, USkeletalMeshComponent* Com, UClass* AnimClass)
{
	Init(FaceId, InMotionType, Com, AnimClass, bReplaceAnimInstance);
}

void UMugenMorphComponent::Init(int32 FaceId, MotionType InMotionType, USkeletalMeshComponent* Com, UClass* AnimClass, bool bInReplaceAnimInstance)
{
	if (!Com)
	{
		UE_LOG(LogSumeruMugenMorph, Error, TEXT("UMugenMorphComponent::Init: Face mesh is null"));
		return;
	}

	ChangeState(EVHState::Idle);
	FaceCom = Com;
	Id = FaceId;
	CurMotionType = InMotionType;
	bReplaceAnimInstance = bInReplaceAnimInstance;
	ARKitMappingDic.Empty();
	LastAppliedBs.Reset();

	if (CurMotionType == MotionType::FLAME)
	{
		BuildFlameNameCache();
	}

	if (CurMotionType == MotionType::ARKit)
	{
		USkeletalMesh* SkeletalMesh = FaceCom->GetSkeletalMeshAsset();
		const int32 MorphCount = (SkeletalMesh) ? SkeletalMesh->GetMorphTargets().Num() : 0;
		if (SkeletalMesh && MorphCount > 0)
		{
			for (UMorphTarget* MorphTarget : SkeletalMesh->GetMorphTargets())
			{
				if (MorphTarget)
				{
					const FName MorphTargetName = MorphTarget->GetFName();
					const FName TempName = GetMappingName(MorphTargetName);
					if (!TempName.IsNone())
					{
						AddMorphTargetAlias(TempName, MorphTargetName);
					}
				}
			}
		}

		int32 AliasHits = 0;
		if (SkeletalMesh)
		{
			TSet<FName> ExistingMorphs;
			for (UMorphTarget* MorphTarget : SkeletalMesh->GetMorphTargets())
			{
				if (MorphTarget)
				{
					ExistingMorphs.Add(MorphTarget->GetFName());
				}
			}
			for (const FName& ARKitName : GetARKitNames())
			{
				TArray<FName> Aliases;
				GetMetaHumanCtrlAliases(ARKitName, Aliases);
				for (const FName& Alias : Aliases)
				{
					if (ExistingMorphs.Contains(Alias))
					{
						AddMorphTargetAlias(ARKitName, Alias);
						++AliasHits;
					}
				}
			}
		}

		UE_LOG(LogSumeruMugenMorph, Log,
			TEXT("UMugenMorphComponent::Init: mesh='%s' morphTargets=%d arkitMapped=%d ctrlAliasHits=%d replaceAnim=%d"),
			*FaceCom->GetName(),
			MorphCount,
			ARKitMappingDic.Num(),
			AliasHits,
			bInReplaceAnimInstance ? 1 : 0);

		if (MorphCount == 0)
		{
			UE_LOG(LogSumeruMugenMorph, Error,
				TEXT("UMugenMorphComponent::Init: mesh '%s' has 0 morph targets. Official MetaHuman faces are RigLogic/CTRL driven; SetMorphTarget will not move the mouth unless the Face mesh has ARKit (or CTRL) morphs."),
				*FaceCom->GetName());
		}
		else if (ARKitMappingDic.Num() == 0)
		{
			UE_LOG(LogSumeruMugenMorph, Warning,
				TEXT("UMugenMorphComponent::Init: no ARKit/CTRL morph name matches on '%s'. Mouth will stay still. Check Output Log and the Face mesh morph list."),
				*FaceCom->GetName());
		}
	}

	BindFaceMeshTick();

	if (bInReplaceAnimInstance)
	{
		if (!AnimClass || !AnimClass->IsChildOf(UMugenMorphAnim::StaticClass()))
		{
			if (AnimClass)
			{
				UE_LOG(LogSumeruMugenMorph, Warning,
					TEXT("UMugenMorphComponent::Init: AnimClass '%s' is not MugenMorphAnim, fallback to C++ class"),
					*AnimClass->GetName());
			}
			AnimClass = ResolveDefaultAnimClass();
			if (!AnimClass || !AnimClass->IsChildOf(UMugenMorphAnim::StaticClass()))
			{
				AnimClass = UMugenMorphAnim::StaticClass();
			}
		}

		FaceCom->SetAnimInstanceClass(AnimClass);
		FaceCom->InitAnim(true);
		MugenMorphAnim = Cast<UMugenMorphAnim>(FaceCom->GetAnimInstance());
		if (!MugenMorphAnim)
		{
			UE_LOG(LogSumeruMugenMorph, Error,
				TEXT("UMugenMorphComponent::Init: failed to create UMugenMorphAnim on mesh '%s'"),
				*FaceCom->GetName());
			return;
		}
		MugenMorphAnim->FaceId = FaceId;
	}
	else
	{
		MugenMorphAnim = Cast<UMugenMorphAnim>(FaceCom->GetAnimInstance());
		if (MugenMorphAnim)
		{
			MugenMorphAnim->FaceId = FaceId;
		}
		else
		{
			UE_LOG(LogSumeruMugenMorph, Warning,
				TEXT("UMugenMorphComponent::Init: keeping AnimBP on '%s' (morph-only path)."),
				*FaceCom->GetName());
		}
	}

	if (AMugenMorphManager* Manager = ResolveManager())
	{
		Manager->RegisterFace(FaceId, this);
	}
	else
	{
		UE_LOG(LogSumeruMugenMorph, Error,
			TEXT("UMugenMorphComponent::Init: AMugenMorphManager missing (world=%s subsystem=%s). Place MugenMorphManager_BP in level or wait until PIE has begun."),
			*GetNameSafe(GetWorld()),
			USumeruMugenMorphSubsystem::Get(this) ? TEXT("ok") : TEXT("null"));
	}
}

void UMugenMorphComponent::CopyLatestARKitWeights(TArray<float>& OutWeights) const
{
	FScopeLock Lock(&FaceCurveLock);
	OutWeights = LatestARKitWeights;
}

FName UMugenMorphComponent::GetMappingName(FName Name) const
{
	return MapMorphNameToARKit(Name);
}

FName UMugenMorphComponent::MapMorphNameToARKit(FName MorphName)
{
	const FString LowerName = MorphName.ToString().ToLower();
	FName BestName;
	int32 BestLen = 0;

	const TArray<FName>& ARKitNames = GetARKitNames();
	for (const FName& TempName : ARKitNames)
	{
		const FString TempString = TempName.ToString().ToLower();
		if (TempString.Len() > BestLen && LowerName.Contains(TempString))
		{
			BestName = TempName;
			BestLen = TempString.Len();
		}
	}

	return BestName;
}

void UMugenMorphComponent::GetMetaHumanCtrlAliases(FName ARKitName, TArray<FName>& OutNames)
{
	static const TMap<FName, TArray<FName>> Aliases = {
		{ FName(TEXT("EyeBlinkLeft")), { FName(TEXT("CTRL_expressions_eyeBlinkL")) } },
		{ FName(TEXT("EyeBlinkRight")), { FName(TEXT("CTRL_expressions_eyeBlinkR")) } },
		{ FName(TEXT("EyeLookDownLeft")), { FName(TEXT("CTRL_expressions_eyeLookDownL")) } },
		{ FName(TEXT("EyeLookDownRight")), { FName(TEXT("CTRL_expressions_eyeLookDownR")) } },
		{ FName(TEXT("EyeLookInLeft")), { FName(TEXT("CTRL_expressions_eyeLookRightL")) } },
		{ FName(TEXT("EyeLookInRight")), { FName(TEXT("CTRL_expressions_eyeLookLeftR")) } },
		{ FName(TEXT("EyeLookOutLeft")), { FName(TEXT("CTRL_expressions_eyeLookLeftL")) } },
		{ FName(TEXT("EyeLookOutRight")), { FName(TEXT("CTRL_expressions_eyeLookRightR")) } },
		{ FName(TEXT("EyeLookUpleft")), { FName(TEXT("CTRL_expressions_eyeLookUpL")) } },
		{ FName(TEXT("EyeLookUpRight")), { FName(TEXT("CTRL_expressions_eyeLookUpR")) } },
		{ FName(TEXT("EyeSquintLeft")), { FName(TEXT("CTRL_expressions_eyeSquintInnerL")) } },
		{ FName(TEXT("EyeSquintRight")), { FName(TEXT("CTRL_expressions_eyeSquintInnerR")) } },
		{ FName(TEXT("EyeWideLeft")), { FName(TEXT("CTRL_expressions_eyeWidenL")) } },
		{ FName(TEXT("EyeWideRight")), { FName(TEXT("CTRL_expressions_eyeWidenR")) } },
		{ FName(TEXT("JawForward")), { FName(TEXT("CTRL_expressions_jawFwd")) } },
		{ FName(TEXT("JawLeft")), { FName(TEXT("CTRL_expressions_jawLeft")) } },
		{ FName(TEXT("JawRight")), { FName(TEXT("CTRL_expressions_jawRight")) } },
		{ FName(TEXT("JawOpen")), { FName(TEXT("CTRL_expressions_jawOpen")) } },
		{ FName(TEXT("MouthClose")), { FName(TEXT("CTRL_expressions_mouthClose")), FName(TEXT("CTRL_expressions_mouthLipsTogetherUL")), FName(TEXT("CTRL_expressions_mouthLipsTogetherUR")) } },
		{ FName(TEXT("MouthFunnel")), { FName(TEXT("CTRL_expressions_mouthFunnelUL")), FName(TEXT("CTRL_expressions_mouthFunnelUR")), FName(TEXT("CTRL_expressions_mouthFunnelDL")), FName(TEXT("CTRL_expressions_mouthFunnelDR")) } },
		{ FName(TEXT("MouthPucker")), { FName(TEXT("CTRL_expressions_mouthLipsPurseUL")), FName(TEXT("CTRL_expressions_mouthLipsPurseUR")), FName(TEXT("CTRL_expressions_mouthLipsPurseDL")), FName(TEXT("CTRL_expressions_mouthLipsPurseDR")) } },
		{ FName(TEXT("MouthLeft")), { FName(TEXT("CTRL_expressions_mouthLeft")) } },
		{ FName(TEXT("MouthRight")), { FName(TEXT("CTRL_expressions_mouthRight")) } },
		{ FName(TEXT("MouthSmileLeft")), { FName(TEXT("CTRL_expressions_mouthSmileL")), FName(TEXT("CTRL_expressions_mouthCornerPullL")), FName(TEXT("CTRL_expressions_mouthSharpCornerPullL")) } },
		{ FName(TEXT("MouthSmileRight")), { FName(TEXT("CTRL_expressions_mouthSmileR")), FName(TEXT("CTRL_expressions_mouthCornerPullR")), FName(TEXT("CTRL_expressions_mouthSharpCornerPullR")) } },
		{ FName(TEXT("MouthFrownLeft")), { FName(TEXT("CTRL_expressions_mouthFrownL")), FName(TEXT("CTRL_expressions_mouthCornerDepressL")) } },
		{ FName(TEXT("MouthFrownRight")), { FName(TEXT("CTRL_expressions_mouthFrownR")), FName(TEXT("CTRL_expressions_mouthCornerDepressR")) } },
		{ FName(TEXT("MouthDimpleLeft")), { FName(TEXT("CTRL_expressions_mouthDimpleL")) } },
		{ FName(TEXT("MouthDimpleRight")), { FName(TEXT("CTRL_expressions_mouthDimpleR")) } },
		{ FName(TEXT("MouthStretchLeft")), { FName(TEXT("CTRL_expressions_mouthStretchL")) } },
		{ FName(TEXT("MouthStretchRight")), { FName(TEXT("CTRL_expressions_mouthStretchR")) } },
		{ FName(TEXT("MouthRollLower")), { FName(TEXT("CTRL_expressions_mouthLowerLipRollInL")), FName(TEXT("CTRL_expressions_mouthLowerLipRollInR")) } },
		{ FName(TEXT("MouthRollUpper")), { FName(TEXT("CTRL_expressions_mouthUpperLipRollInL")), FName(TEXT("CTRL_expressions_mouthUpperLipRollInR")) } },
		{ FName(TEXT("MouthShrugLower")), { FName(TEXT("CTRL_expressions_mouthShrugLower")) } },
		{ FName(TEXT("MouthShrugUpper")), { FName(TEXT("CTRL_expressions_mouthShrugUpper")) } },
		{ FName(TEXT("MouthPressLeft")), { FName(TEXT("CTRL_expressions_mouthPressUL")), FName(TEXT("CTRL_expressions_mouthPressDL")) } },
		{ FName(TEXT("MouthPressRight")), { FName(TEXT("CTRL_expressions_mouthPressUR")), FName(TEXT("CTRL_expressions_mouthPressDR")) } },
		{ FName(TEXT("MouthLowerDownLeft")), { FName(TEXT("CTRL_expressions_mouthLowerLipDepressL")) } },
		{ FName(TEXT("MouthLowerDownRight")), { FName(TEXT("CTRL_expressions_mouthLowerLipDepressR")) } },
		{ FName(TEXT("MouthUpperUpLeft")), { FName(TEXT("CTRL_expressions_mouthUpperLipRaiseL")) } },
		{ FName(TEXT("MouthUpperUpRight")), { FName(TEXT("CTRL_expressions_mouthUpperLipRaiseR")) } },
		{ FName(TEXT("BrowDownLeft")), { FName(TEXT("CTRL_expressions_browDownL")) } },
		{ FName(TEXT("BrowDownRight")), { FName(TEXT("CTRL_expressions_browDownR")) } },
		{ FName(TEXT("BrowInnerUp")), { FName(TEXT("CTRL_expressions_browRaiseInL")), FName(TEXT("CTRL_expressions_browRaiseInR")) } },
		{ FName(TEXT("BrowOuterUpLeft")), { FName(TEXT("CTRL_expressions_browRaiseOuterL")) } },
		{ FName(TEXT("BrowOuterUpRight")), { FName(TEXT("CTRL_expressions_browRaiseOuterR")) } },
		{ FName(TEXT("CheekPuff")), { FName(TEXT("CTRL_expressions_mouthCheekBlowL")), FName(TEXT("CTRL_expressions_mouthCheekBlowR")) } },
		{ FName(TEXT("CheekSquintLeft")), { FName(TEXT("CTRL_expressions_eyeCheekRaiseL")) } },
		{ FName(TEXT("CheekSquintRight")), { FName(TEXT("CTRL_expressions_eyeCheekRaiseR")) } },
		{ FName(TEXT("NoseSneerLeft")), { FName(TEXT("CTRL_expressions_noseWrinkleL")) } },
		{ FName(TEXT("NoseSneerRight")), { FName(TEXT("CTRL_expressions_noseWrinkleR")) } },
		{ FName(TEXT("TongueOut")), { FName(TEXT("CTRL_expressions_tongueOut")) } },
	};

	if (const TArray<FName>* Found = Aliases.Find(ARKitName))
	{
		for (const FName& Name : *Found)
		{
			OutNames.AddUnique(Name);
		}
	}
}

int32 UMugenMorphComponent::CountARKitMorphMatches(const USkeletalMesh* Mesh)
{
	if (!Mesh)
	{
		return 0;
	}

	int32 Mapped = 0;
	for (UMorphTarget* MorphTarget : Mesh->GetMorphTargets())
	{
		if (MorphTarget && !MapMorphNameToARKit(MorphTarget->GetFName()).IsNone())
		{
			++Mapped;
		}
	}
	return Mapped;
}

namespace MugenMorphFaceDetectPrivate
{
	int32 ScoreFaceMesh(USkeletalMeshComponent* Mesh)
	{
		if (!Mesh)
		{
			return MIN_int32;
		}

		int32 Score = 0;
		const FString CompName = Mesh->GetName();
		const bool bNameLooksLikeFace =
			CompName.Contains(TEXT("Face"), ESearchCase::IgnoreCase)
			&& !CompName.Contains(TEXT("Facewear"), ESearchCase::IgnoreCase)
			&& !CompName.Contains(TEXT("FaceBoard"), ESearchCase::IgnoreCase);

		if (bNameLooksLikeFace)
		{
			Score += 1000;
		}

		if (CompName.Contains(TEXT("Hair"), ESearchCase::IgnoreCase)
			|| CompName.Contains(TEXT("Groom"), ESearchCase::IgnoreCase)
			|| CompName.Contains(TEXT("Brow"), ESearchCase::IgnoreCase)
			|| CompName.Contains(TEXT("Lash"), ESearchCase::IgnoreCase)
			|| CompName.Contains(TEXT("Fuzz"), ESearchCase::IgnoreCase)
			|| CompName.Contains(TEXT("Beard"), ESearchCase::IgnoreCase)
			|| CompName.Contains(TEXT("Mustache"), ESearchCase::IgnoreCase))
		{
			Score -= 600;
		}

		USkeletalMesh* Asset = Mesh->GetSkeletalMeshAsset();
		if (Asset)
		{
			const int32 MorphCount = Asset->GetMorphTargets().Num();
			Score += MorphCount * 2;
			Score += UMugenMorphComponent::CountARKitMorphMatches(Asset) * 20;
		}

		return Score;
	}
}

USkeletalMeshComponent* UMugenMorphComponent::FindFaceSkeletalMesh(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	TInlineComponentArray<USkeletalMeshComponent*> Meshes(Actor);
	if (Meshes.Num() == 0)
	{
		return nullptr;
	}
	if (Meshes.Num() == 1)
	{
		return Meshes[0];
	}

	USkeletalMeshComponent* Best = nullptr;
	int32 BestScore = MIN_int32;
	for (USkeletalMeshComponent* Mesh : Meshes)
	{
		const int32 Score = MugenMorphFaceDetectPrivate::ScoreFaceMesh(Mesh);
		if (Score > BestScore)
		{
			BestScore = Score;
			Best = Mesh;
		}
	}
	return Best;
}

USkeletalMeshComponent* UMugenMorphComponent::FindFaceSkeletalMeshInWorld(const UObject* WorldContextObject, AActor* IgnoreActor)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World)
	{
		return nullptr;
	}

	USkeletalMeshComponent* Best = nullptr;
	int32 BestScore = MIN_int32;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor == IgnoreActor
			|| Actor->IsA(AMugenMorphManager::StaticClass())
			|| Actor->IsA(AMugenMorphApiSampleActor::StaticClass()))
		{
			continue;
		}

		USkeletalMeshComponent* Face = FindFaceSkeletalMesh(Actor);
		if (!Face)
		{
			continue;
		}

		int32 Score = MugenMorphFaceDetectPrivate::ScoreFaceMesh(Face);
		if (Actor->FindComponentByClass<UMugenMorphComponent>())
		{
			Score += 150;
		}
		if (Score > BestScore)
		{
			BestScore = Score;
			Best = Face;
		}
	}
	return Best;
}

void UMugenMorphComponent::RandomIdleHandle(float DeltaTime)
{
	AMugenMorphManager* Manager = ResolveManager();
	if (!Manager)
	{
		return;
	}

	if (Manager->Sex == 0)
	{
		const int32 RN = FMath::RandRange(1, 3);
		Manager->PlayInternalEmote(TEXT(""), FString::Printf(TEXT("man-%d.npz"), RN), EMugenMorphDataType::Motion, Id);
	}
	else if (Manager->Sex == 1)
	{
		const int32 RN = FMath::RandRange(1, 7);
		Manager->PlayInternalEmote(TEXT(""), FString::Printf(TEXT("girl-%d.npz"), RN), EMugenMorphDataType::Motion, Id);
	}
}
