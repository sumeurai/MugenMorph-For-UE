// Copyright 2026 Mugen3D. All Rights Reserved.


#include "AnimNode_Mugen3DIK.h"
#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimTrace.h"


FAnimNode_Mugen3DIK::FAnimNode_Mugen3DIK() : Translation(FVector::ZeroVector)
, Rotation(FRotator::ZeroRotator)
, Scale(FVector(1.0f))
, TranslationMode(BMM_Ignore)
, RotationMode(BMM_Ignore)
, ScaleMode(BMM_Ignore)
, TranslationSpace(BCS_ComponentSpace)
, RotationSpace(BCS_ComponentSpace)
, ScaleSpace(BCS_ComponentSpace)
{
	/*for (size_t i = 0; i < 870; i++)
	{
		Translations.Add(FVector(0, 0, 0));
	}

	for (size_t i = 0; i < 870; i++)
	{
		Rotations.Add(FRotator(0, 0, 0));
	}*/
}

void FAnimNode_Mugen3DIK::GatherDebugData(FNodeDebugData& DebugData)
{
   
}

void FAnimNode_Mugen3DIK::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(EvaluateSkeletalControl_AnyThread)
	check(OutBoneTransforms.Num() == 0);

	// the way we apply transform is same as FMatrix or FTransform
	// we apply scale first, and rotation, and translation
	// if you'd like to translate first, you'll need two nodes that first node does translate and second nodes to rotate.
	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();

	TArray<FCompactPoseBoneIndex> CompactPoseBonesToModify;

	int BonesNum = BoneContainer.GetNumBones();

	for (int32 i = 0; i < BonesNum; i++)
	{
		CompactPoseBonesToModify.Add(FCompactPoseBoneIndex(i));
	}

	//FCompactPoseBoneIndex CompactPoseBoneToModify = BoneToModify.GetCompactPoseIndex(BoneContainer);
	//FTransform NewBoneTM = Output.Pose.GetComponentSpaceTransform(CompactPoseBoneToModify);
	FTransform ComponentTransform = Output.AnimInstanceProxy->GetComponentTransform();

	//if (ScaleMode != BMM_Ignore)
	//{
	//	// Convert to Bone Space.
	//	FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, ScaleSpace);

	//	if (ScaleMode == BMM_Additive)
	//	{
	//		NewBoneTM.SetScale3D(NewBoneTM.GetScale3D() * Scale);
	//	}
	//	else
	//	{
	//		NewBoneTM.SetScale3D(Scale);
	//	}

	//	// Convert back to Component Space.
	//	FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, ScaleSpace);
	//}
	for (size_t i = 0; i < BonesNum; i++)
	{

		FCompactPoseBoneIndex CompactPoseBoneToModify = CompactPoseBonesToModify[i];
		FTransform NewBoneTM = Output.Pose.GetComponentSpaceTransform(CompactPoseBoneToModify);
		// Convert to Bone Space.
		FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, TranslationSpace);
		
		if (RotationMode != BMM_Ignore)
		{
			const FQuat BoneQuat(Rotation);
			if (RotationMode == BMM_Additive)
			{
				NewBoneTM.SetRotation(BoneQuat * NewBoneTM.GetRotation());
			}
			else
			{
				NewBoneTM.SetRotation(BoneQuat);
			}
		}

		if (TranslationMode != BMM_Ignore)
		{
			if (TranslationMode == BMM_Additive)
			{
				if (ModifyData.Translations.Num() > 0)
				{
					NewBoneTM.AddToTranslation(ModifyData.Translations[i]);
					/*if (ModifyData.Translations.Num() == BonesNum)
					{
						UE_LOG(LogTemp, Display, TEXT(" %s"), "here2");
					}*/
				}
				else
				{
					NewBoneTM.AddToTranslation(Translation);
					//UE_LOG(LogTemp, Display, TEXT(" %s"), "here3");
				}
			}
			else
			{
				if (ModifyData.Translations.Num() > 0)
				{
					NewBoneTM.SetTranslation(ModifyData.Translations[i]);
				}
				else
				{
					NewBoneTM.SetTranslation(Translation);
				}
			}
		}

		// Convert back to Component Space.
		FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, RotationSpace);
		OutBoneTransforms.Add(FBoneTransform(CompactPoseBoneToModify, NewBoneTM));
	}
		
				
	

	//if (TranslationMode != BMM_Ignore)
	//{
	//	// Convert to Bone Space.
	//	FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, TranslationSpace);

	//	if (TranslationMode == BMM_Additive)
	//	{
	//		NewBoneTM.AddToTranslation(Translation);
	//	}
	//	else
	//	{
	//		NewBoneTM.SetTranslation(Translation);
	//	}

	//	// Convert back to Component Space.
	//	FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, TranslationSpace);
	//}

	

	TRACE_ANIM_NODE_VALUE(Output, TEXT("Target"), BoneToModify.BoneName);
}

bool FAnimNode_Mugen3DIK::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	// if both bones are valid
	return (BoneToModify.IsValidToEvaluate(RequiredBones));
}

void FAnimNode_Mugen3DIK::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(InitializeBoneReferences)
	BoneToModify.Initialize(RequiredBones);
}

