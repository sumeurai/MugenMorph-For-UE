// Copyright 2026 SumeruAI. All Rights Reserved.

#include "AnimNode_SumeruAIAudioToFace.h"

#include "ILiveLinkClient.h"

#include "Features/IModularFeatures.h"
#include "../../SumeruAIATF/Public/AudioToFaceAnim.h"
#include "Animation/AnimInstanceProxy.h"
#include "LiveLinkCustomVersion.h"
#include "LiveLinkRemapAsset.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "Animation/AnimTrace.h"
#include "../../SumeruAIATF/Public/AudioToFace.h"

enum SumeruAIBS
{
	EyeBlinkLeft,
	EyeLookDownLeft,
	EyeLookInLeft,
	EyeLookOutLeft,
	EyeLookUpleft,
	EyeSquintLeft,
	EyeWideLeft,
	EyeBlinkRight,
	EyeLookDownRight,
	EyeLookInRight,
	EyeLookOutRight,
	EyeLookUpRight,
	EyeSquintRight,
	EyeWideRight,
	JawForward,
	JawLeft,
	JawRight,
	JawOpen,
	MouthClose,
	MouthFunnel,
	MouthPucker,
	MouthLeft,
	MouthRight,
	MouthSmileLeft,
	MouthSmileRight,
	MouthFrownLeft,
	MouthFrownRight,
	MouthDimpleLeft,
	MouthDimpleRight,
	MouthStretchLeft,
	MouthStretchRight,
	MouthRollLower,
	MouthRollUpper,
	MouthShrugLower,
	MouthShrugUpper,
	MouthPressLeft,
	MouthPressRight,
	MouthLowerDownLeft,
	MouthLowerDownRight,
	MouthUpperUpLeft,
	MouthUpperUpRight,
	BrowDownLeft,
	BrowDownRight,
	BrowInnerUp,
	BrowOuterUpLeft,
	BrowOuterUpRight,
	CheekPuff,
	CheekSquintLeft,
	CheekSquintRight,
	NoseSneerLeft,
	NoseSneerRight,
	TongueOut,
	HeadYaw,
	HeadPitch,
	HeadRoll,
	LeftEyeYaw,
	LeftEyePitch,
	LeftEyeRoll,
	RightEyeYaw,
	RightEyePitch,
	RightEyeRoll,
};

FString EnumToString(SumeruAIBS BS) {
	switch (BS) {
	case SumeruAIBS::EyeBlinkLeft:   return "EyeBlinkLeft";
	case SumeruAIBS::EyeLookDownLeft: return "EyeLookDownLeft";
	case SumeruAIBS::EyeLookInLeft:  return "EyeLookInLeft";
	case SumeruAIBS::EyeLookOutLeft: return "EyeLookOutLeft";
	case SumeruAIBS::EyeLookUpleft: return "EyeLookUpleft";
	case SumeruAIBS::EyeSquintLeft: return "EyeSquintLeft";
	case SumeruAIBS::EyeWideLeft: return "EyeWideLeft";
	case SumeruAIBS::EyeBlinkRight: return "EyeBlinkRight";
	case SumeruAIBS::EyeLookDownRight: return "EyeLookDownRight";
	case SumeruAIBS::EyeLookInRight: return "EyeLookInRight";
	case SumeruAIBS::EyeLookOutRight: return "EyeLookOutRight";
	case SumeruAIBS::EyeLookUpRight: return "EyeLookUpRight";
	case SumeruAIBS::EyeSquintRight: return "EyeSquintRight";
	case SumeruAIBS::EyeWideRight: return "EyeWideRight";
	case SumeruAIBS::JawForward:   return "JawForward";
	case SumeruAIBS::JawLeft: return "JawLeft";
	case SumeruAIBS::JawRight:  return "JawRight";
	case SumeruAIBS::JawOpen: return "JawOpen";
	case SumeruAIBS::MouthClose: return "MouthClose";
	case SumeruAIBS::MouthFunnel: return "MouthFunnel";
	case SumeruAIBS::MouthPucker: return "MouthPucker";
	case SumeruAIBS::MouthLeft: return "MouthLeft";
	case SumeruAIBS::MouthRight: return "MouthRight";
	case SumeruAIBS::MouthSmileLeft: return "MouthSmileLeft";
	case SumeruAIBS::MouthSmileRight: return "MouthSmileRight";
	case SumeruAIBS::MouthFrownLeft: return "MouthFrownLeft";
	case SumeruAIBS::MouthFrownRight: return "MouthFrownRight";
	case SumeruAIBS::MouthDimpleLeft: return "MouthDimpleLeft";
	case SumeruAIBS::MouthDimpleRight: return "MouthDimpleRight";
	case SumeruAIBS::MouthStretchLeft:   return "MouthStretchLeft";
	case SumeruAIBS::MouthStretchRight: return "MouthStretchRight";
	case SumeruAIBS::MouthRollLower:  return "MouthRollLower";
	case SumeruAIBS::MouthRollUpper: return "MouthRollUpper";
	case SumeruAIBS::MouthShrugLower: return "MouthShrugLower";
	case SumeruAIBS::MouthShrugUpper: return "MouthShrugUpper";
	case SumeruAIBS::MouthPressLeft: return "MouthPressLeft";
	case SumeruAIBS::MouthPressRight: return "MouthPressRight";
	case SumeruAIBS::MouthLowerDownLeft: return "MouthLowerDownLeft";
	case SumeruAIBS::MouthLowerDownRight: return "MouthLowerDownRight";
	case SumeruAIBS::MouthUpperUpLeft: return "MouthUpperUpLeft";
	case SumeruAIBS::MouthUpperUpRight: return "MouthUpperUpRight";
	case SumeruAIBS::BrowDownLeft: return "BrowDownLeft";
	case SumeruAIBS::BrowDownRight: return "BrowDownRight";
	case SumeruAIBS::BrowInnerUp:   return "BrowInnerUp";
	case SumeruAIBS::BrowOuterUpLeft: return "BrowOuterUpLeft";
	case SumeruAIBS::BrowOuterUpRight:  return "BrowOuterUpRight";
	case SumeruAIBS::CheekPuff: return "CheekPuff";
	case SumeruAIBS::CheekSquintLeft: return "CheekSquintLeft";
	case SumeruAIBS::CheekSquintRight: return "CheekSquintRight";
	case SumeruAIBS::NoseSneerLeft: return "NoseSneerLeft";
	case SumeruAIBS::NoseSneerRight: return "NoseSneerRight";
	case SumeruAIBS::TongueOut: return "TongueOut";
	case SumeruAIBS::HeadYaw: return "HeadYaw";
	case SumeruAIBS::HeadPitch: return "HeadPitch";
	case SumeruAIBS::HeadRoll: return "HeadRoll";
	case SumeruAIBS::LeftEyeYaw: return "LeftEyeYaw";
	case SumeruAIBS::LeftEyePitch: return "LeftEyePitch";
	case SumeruAIBS::LeftEyeRoll: return "LeftEyeRoll";
	case SumeruAIBS::RightEyeYaw: return "RightEyeYaw";
	case SumeruAIBS::RightEyePitch: return "RightEyePitch";
	case SumeruAIBS::RightEyeRoll: return "RightEyeRoll";
	default:           return "UNKNOWN";
	}
}

FAnimNode_SumeruAIAudioToFace::FAnimNode_SumeruAIAudioToFace()
	: RetargetAsset(ULiveLinkRemapAsset::StaticClass())
	, CurrentRetargetAsset(nullptr)
	, LiveLinkClient_AnyThread(nullptr)
	, CachedDeltaTime(0.0f)
{
}

void FAnimNode_SumeruAIAudioToFace::OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance)
{
	CurrentRetargetAsset = nullptr;
	
	Super::OnInitializeAnimInstance(InProxy, InAnimInstance);

	
}

void FAnimNode_SumeruAIAudioToFace::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	InputPose.Initialize(Context);
	PlayInterval = 0;
	Weight = 1;
	Direction = -1;
}

void FAnimNode_SumeruAIAudioToFace::PreUpdate(const UAnimInstance* InAnimInstance)
{
	LiveLinkClient_AnyThread = LiveLinkClient_GameThread.GetClient();

	
	// Protection as a class graph pin does not honor rules on abstract classes and NoClear
	UClass* RetargetAssetPtr = RetargetAsset.Get();
	if (!RetargetAssetPtr || RetargetAssetPtr->HasAnyClassFlags(CLASS_Abstract))
	{
		RetargetAssetPtr = ULiveLinkRemapAsset::StaticClass();
		RetargetAsset = RetargetAssetPtr;
	}

	if (!CurrentRetargetAsset || RetargetAssetPtr != CurrentRetargetAsset->GetClass())
	{
		CurrentRetargetAsset = NewObject<ULiveLinkRetargetAsset>(const_cast<UAnimInstance*>(InAnimInstance), RetargetAssetPtr);
		CurrentRetargetAsset->Initialize();
	}


	/*if (!CurAnimInstance && AAudioToFaceManager::Instance() && AAudioToFaceManager::Instance()->IsPlay)
	{
		CurAnimInstance = InAnimInstance;
		AAudioToFaceManager::Instance()->RegisterAnim(InAnimInstance);
	}*/
}

void FAnimNode_SumeruAIAudioToFace::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	InputPose.Update(Context);
	


	GetEvaluateGraphExposedInputs().Execute(Context);

	// Accumulate Delta time from update
	CachedDeltaTime += Context.GetDeltaTime();
	//UE_LOG(LogTemp, Display, TEXT(" %s"), *FString::SanitizeFloat(CachedDeltaTime));
	TRACE_ANIM_NODE_VALUE(Context, TEXT("SubjectName"), LiveLinkSubjectName.Name);
}

void FAnimNode_SumeruAIAudioToFace::Evaluate_AnyThread(FPoseContext& Output)
{
	InputPose.Evaluate(Output);

	/*if (!LiveLinkClient_AnyThread || !CurrentRetargetAsset)
	{
		return;
	}*/

	//if (!AAudioToFaceManager::Instance())
	//	return;

	//if (!AAudioToFaceManager::Instance()->IsPlay)
	//	return;

	//FAudioExpressionData &EmoteData1 = AAudioToFaceManager::Instance()->EmoteData1;
	//FAudioExpressionData &EmoteData2 = AAudioToFaceManager::Instance()->EmoteData2;

	////UE_LOG(LogTemp, Display, TEXT(" ========before"));
	//if (!EmoteData1.Expressions.IsEmpty() || !EmoteData2.Expressions.IsEmpty())
	//{

	//	FAudioCacheData TempData;
	//	FAudioExpression tempData1;
	//	FAudioExpression tempData2;
	//	FAudioExpression finalData;
	//	
	//	EmoteData1.GetCurFrame(CachedDeltaTime);
	//	EmoteData2.GetCurFrame(CachedDeltaTime);

	////	UE_LOG(LogTemp, Display, TEXT(" %s"), *FString::FromInt(EmoteData1.CurFrame));
	////	UE_LOG(LogTemp, Display, TEXT(" %s"), *FString::SanitizeFloat(EmoteData2.UsedTime));
	////	UE_LOG(LogTemp, Display, TEXT(" ========after"));
	//	if (!EmoteData1.Expressions.IsEmpty() && EmoteData1.CurFrame < EmoteData1.Expressions.Num())
	//	{
	//		tempData1 = EmoteData1.Expressions[EmoteData1.CurFrame];
	//	}

	//	if (!EmoteData2.Expressions.IsEmpty() && EmoteData2.CurFrame < EmoteData2.Expressions.Num())
	//	{
	//		tempData2 = EmoteData2.Expressions[EmoteData2.CurFrame];	
	//	}

	//	if (!tempData1.Data.IsEmpty() && tempData2.Data.IsEmpty())
	//	{
	//		finalData = tempData1;
	//	}
	//	else if (!tempData1.Data.IsEmpty() && !tempData2.Data.IsEmpty())
	//	{
	//		finalData.Data.Reserve(61);	
	//		for (int i = 0; i < tempData1.Data.Num(); i++)
	//		{
	//			finalData.Data.Add(tempData1.Data[i] * Weight + tempData2.Data[i] * (1 - Weight));
	//		}

	//		Weight += Direction * CachedDeltaTime * 2;
	//		Weight = FMath::Clamp(Weight, 0, 1);
	//	   
	//		if (Weight == 0)
	//		{
	//			EmoteData1.Reset();
	//			Direction = 1;
	//		}
	//		else if(Weight == 1)
	//		{
	//			EmoteData2.Reset();
	//			Direction = -1;
	//		}
	//	}
	//	else if (tempData1.Data.IsEmpty() && !tempData2.Data.IsEmpty())
	//	{
	//		finalData = tempData2;
	//	}
	//	else if (tempData1.Data.IsEmpty() && tempData2.Data.IsEmpty())
	//	{
	//		EmoteData1.Reset();
	//		EmoteData2.Reset();
	//	//	UE_LOG(LogTemp, Display, TEXT("=============end"));
	//		TempData.Reset();
	//		CurFrame.Empty();
	//		Weight = 1;
	//		Direction = -1;
	//		AAudioToFaceManager::Instance()->UpdateAnimData(CurAnimInstance, TempData);		
	//		AAudioToFaceManager::Instance()->CheckSpeechOver();
	//		return;
	//	}
	//	
	//	
	//
	if(EmoteData.Data.Num() == 0)
		return;

	// EmoteData.Data.Empty();
	// for (size_t i = 0; i < 61; i++)
	// {
	// 	float value =0;
	// 	if(i==0)
	// 		value = 1;
	// 		
	// 	EmoteData.Data.Add(value);
	// 	
	// }

	//UE_LOG(LogTemp, Display, TEXT("=============end %s"), *FString::FromInt(EmoteData.Data.Num()));
	
			FLiveLinkBaseStaticData BaseStaticData;
			FLiveLinkBaseFrameData BaseFrameData;

			for (size_t i = 0; i < EmoteData.Data.Num(); i++)
			{
				float TempValue = 0;
				TempValue = EmoteData.Data[i];
				// switch (i)
				// {
				// case SumeruAIBS::MouthClose:
				// 	TempValue = BsCurve->GetFloatValue(EmoteData[i]);
				// 	break;
				// case SumeruAIBS::JawOpen:
				// 	TempValue = BsCurve->GetFloatValue(finalData.Data[i]);
				// 	TempData.MaleJawOpenOffset = -3.55 * 0.5 * TempValue;
				// 	TempData.FemaleJawOpenOffset = -3.07 * 0.5 * TempValue;
				// 	break;
				//
				// default:
				// 	TempValue = finalData.Data[i];
				// 	break;
				// }

				BaseFrameData.PropertyValues.Add(TempValue);
				FName Name = FName(EnumToString((SumeruAIBS)i));

				// if ((SumeruAIBS)i == SumeruAIBS::HeadRoll)
				// {
				// 	TempData.HeadRoll = finalData.Data[i];
				// }
				// else if ((SumeruAIBS)i == SumeruAIBS::HeadPitch)
				// {
				// 	TempData.HeadPitch = finalData.Data[i];
				// }
				// else if ((SumeruAIBS)i == SumeruAIBS::HeadYaw)
				// {
				// 	TempData.HeadYaw = finalData.Data[i];
				// }

				BaseStaticData.PropertyNames.Add(Name);


			}

			//TempData.StartEmote = true;
			//AAudioToFaceManager::Instance()->UpdateAnimData(CurAnimInstance, TempData);
			//	UE_LOG(LogTemp, Display, TEXT(" %s"), *FString::SanitizeFloat(CachedDeltaTime));
			CurrentRetargetAsset->BuildPoseAndCurveFromBaseData(CachedDeltaTime, &BaseStaticData, &BaseFrameData, Output.Pose, Output.Curve);
	//		//	CachedDeltaTime = 0.f; // Reset so that if we evaluate again we don't "create" time inside of the retargeter

			CachedDeltaTime = 0;
	//	//	UE_LOG(LogTemp, Display, TEXT(" %s"), *FString("start play Evaluate_AnyThread"));

	//		
	//		
	//}
	//else
	//{
	//	
	//	CachedDeltaTime = 0;
	//}

	//FLiveLinkSubjectFrameData SubjectFrameData;

	//TSubclassOf<ULiveLinkRole> SubjectRole = LiveLinkClient_AnyThread->GetSubjectRole(LiveLinkSubjectName);
	//if (SubjectRole)
	//{
	//	if (LiveLinkClient_AnyThread->DoesSubjectSupportsRole(LiveLinkSubjectName, ULiveLinkAnimationRole::StaticClass()))
	//	{
	//		//Process animation data if the subject is from that type
	//		if (LiveLinkClient_AnyThread->EvaluateFrame_AnyThread(LiveLinkSubjectName, ULiveLinkAnimationRole::StaticClass(), SubjectFrameData))
	//		{
	//			FLiveLinkSkeletonStaticData* SkeletonData = SubjectFrameData.StaticData.Cast<FLiveLinkSkeletonStaticData>();
	//			FLiveLinkAnimationFrameData* FrameData = SubjectFrameData.FrameData.Cast<FLiveLinkAnimationFrameData>();
	//			check(SkeletonData);
	//			check(FrameData);

	//			CurrentRetargetAsset->BuildPoseFromAnimationData(CachedDeltaTime, SkeletonData, FrameData, Output.Pose);
	//			CurrentRetargetAsset->BuildPoseAndCurveFromBaseData(CachedDeltaTime, SkeletonData, FrameData, Output.Pose, Output.Curve);
	//			CachedDeltaTime = 0.f; // Reset so that if we evaluate again we don't "create" time inside of the retargeter
	//		}
	//	}
	//	else if (LiveLinkClient_AnyThread->DoesSubjectSupportsRole(LiveLinkSubjectName, ULiveLinkBasicRole::StaticClass()))
	//	{
	//		//Otherwise, fetch basic data that contains property / curve data
	//		if (LiveLinkClient_AnyThread->EvaluateFrame_AnyThread(LiveLinkSubjectName, ULiveLinkBasicRole::StaticClass(), SubjectFrameData))
	//		{
	//			FLiveLinkBaseStaticData* BaseStaticData = SubjectFrameData.StaticData.Cast<FLiveLinkBaseStaticData>();
	//			FLiveLinkBaseFrameData* BaseFrameData = SubjectFrameData.FrameData.Cast<FLiveLinkBaseFrameData>();

	//			if (StartExpression && CustomExpression.Expressions.Num() > 0)
	//			{
	//				if (FrameCount < CustomExpression.Expressions.Num())
	//				{
	//					auto CurFrameData = CustomExpression.Expressions[FrameCount];
	//					for (size_t i = 0; i < BaseFrameData->PropertyValues.Num(); i++)
	//					{
	//						BaseFrameData->PropertyValues[i] = CurFrameData.Data[i];
	//					}
	//					FrameCount++;
	//				}
	//				else
	//				{
	//					FrameCount = 0;
	//					StartExpression = false;
	//					CustomExpression.Expressions.Empty();
	//				}
	//			}



	//			check(BaseStaticData);
	//			check(BaseFrameData);

	//			CurrentRetargetAsset->BuildPoseAndCurveFromBaseData(CachedDeltaTime, BaseStaticData, BaseFrameData, Output.Pose, Output.Curve);
	//			CachedDeltaTime = 0.f; // Reset so that if we evaluate again we don't "create" time inside of the retargeter
	//		}		
	//	}
	//}
}

void FAnimNode_SumeruAIAudioToFace::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	Super::CacheBones_AnyThread(Context);
	InputPose.CacheBones(Context);
}

void FAnimNode_SumeruAIAudioToFace::GatherDebugData(FNodeDebugData& DebugData)
{
	FString DebugLine = FString::Printf(TEXT("LiveLink - SubjectName: %s"), *LiveLinkSubjectName.ToString());

	DebugData.AddDebugItem(DebugLine);
	InputPose.GatherDebugData(DebugData);
}

bool FAnimNode_SumeruAIAudioToFace::Serialize(FArchive& Ar)
{
	Ar.UsingCustomVersion(FLiveLinkCustomVersion::GUID);

	UScriptStruct* Struct = FAnimNode_SumeruAIAudioToFace::StaticStruct();

	if (Ar.IsLoading() || Ar.IsSaving())
	{
		Struct->SerializeTaggedProperties(Ar, (uint8*)this, Struct, nullptr);
	}

#if WITH_EDITORONLY_DATA
	//Take old data and put it in new data structure
	if (Ar.IsLoading())
	{
		if (Ar.CustomVer(FLiveLinkCustomVersion::GUID) < FLiveLinkCustomVersion::NewLiveLinkRoleSystem)
		{
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
				LiveLinkSubjectName.Name = SubjectName_DEPRECATED;
			PRAGMA_ENABLE_DEPRECATION_WARNINGS
		}
	}
#endif

	return true;
}