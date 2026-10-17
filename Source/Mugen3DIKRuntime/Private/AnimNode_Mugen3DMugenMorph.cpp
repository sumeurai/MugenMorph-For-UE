// Copyright 2026 Mugen3D. All Rights Reserved.

#include "AnimNode_Mugen3DMugenMorph.h"

#include "ILiveLinkClient.h"

#include "Features/IModularFeatures.h"
#include "../../Mugen3DMugenMorph/Public/MugenMorphAnim.h"
#include "Animation/AnimInstanceProxy.h"
#include "LiveLinkCustomVersion.h"
#include "LiveLinkRemapAsset.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "Animation/AnimTrace.h"
#include "../../Mugen3DMugenMorph/Public/MugenMorph.h"

enum Mugen3DBS
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

FString EnumToString(Mugen3DBS BS) {
	switch (BS) {
	case Mugen3DBS::EyeBlinkLeft:   return "EyeBlinkLeft";
	case Mugen3DBS::EyeLookDownLeft: return "EyeLookDownLeft";
	case Mugen3DBS::EyeLookInLeft:  return "EyeLookInLeft";
	case Mugen3DBS::EyeLookOutLeft: return "EyeLookOutLeft";
	case Mugen3DBS::EyeLookUpleft: return "EyeLookUpleft";
	case Mugen3DBS::EyeSquintLeft: return "EyeSquintLeft";
	case Mugen3DBS::EyeWideLeft: return "EyeWideLeft";
	case Mugen3DBS::EyeBlinkRight: return "EyeBlinkRight";
	case Mugen3DBS::EyeLookDownRight: return "EyeLookDownRight";
	case Mugen3DBS::EyeLookInRight: return "EyeLookInRight";
	case Mugen3DBS::EyeLookOutRight: return "EyeLookOutRight";
	case Mugen3DBS::EyeLookUpRight: return "EyeLookUpRight";
	case Mugen3DBS::EyeSquintRight: return "EyeSquintRight";
	case Mugen3DBS::EyeWideRight: return "EyeWideRight";
	case Mugen3DBS::JawForward:   return "JawForward";
	case Mugen3DBS::JawLeft: return "JawLeft";
	case Mugen3DBS::JawRight:  return "JawRight";
	case Mugen3DBS::JawOpen: return "JawOpen";
	case Mugen3DBS::MouthClose: return "MouthClose";
	case Mugen3DBS::MouthFunnel: return "MouthFunnel";
	case Mugen3DBS::MouthPucker: return "MouthPucker";
	case Mugen3DBS::MouthLeft: return "MouthLeft";
	case Mugen3DBS::MouthRight: return "MouthRight";
	case Mugen3DBS::MouthSmileLeft: return "MouthSmileLeft";
	case Mugen3DBS::MouthSmileRight: return "MouthSmileRight";
	case Mugen3DBS::MouthFrownLeft: return "MouthFrownLeft";
	case Mugen3DBS::MouthFrownRight: return "MouthFrownRight";
	case Mugen3DBS::MouthDimpleLeft: return "MouthDimpleLeft";
	case Mugen3DBS::MouthDimpleRight: return "MouthDimpleRight";
	case Mugen3DBS::MouthStretchLeft:   return "MouthStretchLeft";
	case Mugen3DBS::MouthStretchRight: return "MouthStretchRight";
	case Mugen3DBS::MouthRollLower:  return "MouthRollLower";
	case Mugen3DBS::MouthRollUpper: return "MouthRollUpper";
	case Mugen3DBS::MouthShrugLower: return "MouthShrugLower";
	case Mugen3DBS::MouthShrugUpper: return "MouthShrugUpper";
	case Mugen3DBS::MouthPressLeft: return "MouthPressLeft";
	case Mugen3DBS::MouthPressRight: return "MouthPressRight";
	case Mugen3DBS::MouthLowerDownLeft: return "MouthLowerDownLeft";
	case Mugen3DBS::MouthLowerDownRight: return "MouthLowerDownRight";
	case Mugen3DBS::MouthUpperUpLeft: return "MouthUpperUpLeft";
	case Mugen3DBS::MouthUpperUpRight: return "MouthUpperUpRight";
	case Mugen3DBS::BrowDownLeft: return "BrowDownLeft";
	case Mugen3DBS::BrowDownRight: return "BrowDownRight";
	case Mugen3DBS::BrowInnerUp:   return "BrowInnerUp";
	case Mugen3DBS::BrowOuterUpLeft: return "BrowOuterUpLeft";
	case Mugen3DBS::BrowOuterUpRight:  return "BrowOuterUpRight";
	case Mugen3DBS::CheekPuff: return "CheekPuff";
	case Mugen3DBS::CheekSquintLeft: return "CheekSquintLeft";
	case Mugen3DBS::CheekSquintRight: return "CheekSquintRight";
	case Mugen3DBS::NoseSneerLeft: return "NoseSneerLeft";
	case Mugen3DBS::NoseSneerRight: return "NoseSneerRight";
	case Mugen3DBS::TongueOut: return "TongueOut";
	case Mugen3DBS::HeadYaw: return "HeadYaw";
	case Mugen3DBS::HeadPitch: return "HeadPitch";
	case Mugen3DBS::HeadRoll: return "HeadRoll";
	case Mugen3DBS::LeftEyeYaw: return "LeftEyeYaw";
	case Mugen3DBS::LeftEyePitch: return "LeftEyePitch";
	case Mugen3DBS::LeftEyeRoll: return "LeftEyeRoll";
	case Mugen3DBS::RightEyeYaw: return "RightEyeYaw";
	case Mugen3DBS::RightEyePitch: return "RightEyePitch";
	case Mugen3DBS::RightEyeRoll: return "RightEyeRoll";
	default:           return "UNKNOWN";
	}
}

FAnimNode_Mugen3DMugenMorph::FAnimNode_Mugen3DMugenMorph()
	: RetargetAsset(ULiveLinkRemapAsset::StaticClass())
	, CurrentRetargetAsset(nullptr)
	, LiveLinkClient_AnyThread(nullptr)
	, CachedDeltaTime(0.0f)
{
}

void FAnimNode_Mugen3DMugenMorph::OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance)
{
	CurrentRetargetAsset = nullptr;
	
	Super::OnInitializeAnimInstance(InProxy, InAnimInstance);

	
}

void FAnimNode_Mugen3DMugenMorph::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	InputPose.Initialize(Context);
	PlayInterval = 0;
	Weight = 1;
	Direction = -1;
}

void FAnimNode_Mugen3DMugenMorph::PreUpdate(const UAnimInstance* InAnimInstance)
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


	/*if (!CurAnimInstance && AMugenMorphManager::Instance() && AMugenMorphManager::Instance()->IsPlay)
	{
		CurAnimInstance = InAnimInstance;
		AMugenMorphManager::Instance()->RegisterAnim(InAnimInstance);
	}*/
}

void FAnimNode_Mugen3DMugenMorph::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	InputPose.Update(Context);
	


	GetEvaluateGraphExposedInputs().Execute(Context);

	// Accumulate Delta time from update
	CachedDeltaTime += Context.GetDeltaTime();
	//UE_LOG(LogTemp, Display, TEXT(" %s"), *FString::SanitizeFloat(CachedDeltaTime));
	TRACE_ANIM_NODE_VALUE(Context, TEXT("SubjectName"), LiveLinkSubjectName.Name);
}

void FAnimNode_Mugen3DMugenMorph::Evaluate_AnyThread(FPoseContext& Output)
{
	InputPose.Evaluate(Output);

	/*if (!LiveLinkClient_AnyThread || !CurrentRetargetAsset)
	{
		return;
	}*/

	//if (!AMugenMorphManager::Instance())
	//	return;

	//if (!AMugenMorphManager::Instance()->IsPlay)
	//	return;

	//FAudioExpressionData &EmoteData1 = AMugenMorphManager::Instance()->EmoteData1;
	//FAudioExpressionData &EmoteData2 = AMugenMorphManager::Instance()->EmoteData2;

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
	//		AMugenMorphManager::Instance()->UpdateAnimData(CurAnimInstance, TempData);		
	//		AMugenMorphManager::Instance()->CheckSpeechOver();
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
				// case Mugen3DBS::MouthClose:
				// 	TempValue = BsCurve->GetFloatValue(EmoteData[i]);
				// 	break;
				// case Mugen3DBS::JawOpen:
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
				FName Name = FName(EnumToString((Mugen3DBS)i));

				// if ((Mugen3DBS)i == Mugen3DBS::HeadRoll)
				// {
				// 	TempData.HeadRoll = finalData.Data[i];
				// }
				// else if ((Mugen3DBS)i == Mugen3DBS::HeadPitch)
				// {
				// 	TempData.HeadPitch = finalData.Data[i];
				// }
				// else if ((Mugen3DBS)i == Mugen3DBS::HeadYaw)
				// {
				// 	TempData.HeadYaw = finalData.Data[i];
				// }

				BaseStaticData.PropertyNames.Add(Name);


			}

			//TempData.StartEmote = true;
			//AMugenMorphManager::Instance()->UpdateAnimData(CurAnimInstance, TempData);
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

void FAnimNode_Mugen3DMugenMorph::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	Super::CacheBones_AnyThread(Context);
	InputPose.CacheBones(Context);
}

void FAnimNode_Mugen3DMugenMorph::GatherDebugData(FNodeDebugData& DebugData)
{
	FString DebugLine = FString::Printf(TEXT("LiveLink - SubjectName: %s"), *LiveLinkSubjectName.ToString());

	DebugData.AddDebugItem(DebugLine);
	InputPose.GatherDebugData(DebugData);
}

bool FAnimNode_Mugen3DMugenMorph::Serialize(FArchive& Ar)
{
	Ar.UsingCustomVersion(FLiveLinkCustomVersion::GUID);

	UScriptStruct* Struct = FAnimNode_Mugen3DMugenMorph::StaticStruct();

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