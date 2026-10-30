// Copyright 2026 Mugen3D. All Rights Reserved.

#include "Animation/AnimNode_MugenMorphFaceSync.h"

#include "MugenMorphComponent.h"
#include "Animation/AnimCurveUtils.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

FAnimNode_MugenMorphFaceSync::FAnimNode_MugenMorphFaceSync()
{
}

void FAnimNode_MugenMorphFaceSync::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Initialize_AnyThread)
	SourcePose.Initialize(Context);
	ResolvedComponent = nullptr;
}

void FAnimNode_MugenMorphFaceSync::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(CacheBones_AnyThread)
	SourcePose.CacheBones(Context);
}

void FAnimNode_MugenMorphFaceSync::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Update_AnyThread)
	SourcePose.Update(Context);
}

void FAnimNode_MugenMorphFaceSync::PreUpdate(const UAnimInstance* InAnimInstance)
{
	if (MugenMorphComponent && IsValid(MugenMorphComponent))
	{
		ResolvedComponent = MugenMorphComponent;
		return;
	}

	if (ResolvedComponent.IsValid())
	{
		return;
	}

	if (!InAnimInstance)
	{
		return;
	}

	AActor* Owner = InAnimInstance->GetOwningActor();
	if (!Owner)
	{
		return;
	}

	UMugenMorphComponent* Found = Owner->FindComponentByClass<UMugenMorphComponent>();
	if (!Found)
	{
		if (AActor* Parent = Owner->GetAttachParentActor())
		{
			Found = Parent->FindComponentByClass<UMugenMorphComponent>();
		}
	}
	ResolvedComponent = Found;
}

void FAnimNode_MugenMorphFaceSync::Evaluate_AnyThread(FPoseContext& Output)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Evaluate_AnyThread)
	SourcePose.Evaluate(Output);

	UMugenMorphComponent* Component = ResolvedComponent.Get();
	if (!Component)
	{
		return;
	}

	TArray<float> Weights;
	Component->CopyLatestARKitWeights(Weights);
	if (Weights.Num() == 0)
	{
		return;
	}

	const TArray<FName>& ARKitNames = UMugenMorphComponent::GetARKitNames();
	const int32 Count = FMath::Min(Weights.Num(), ARKitNames.Num());

	TMap<FName, float> Curves;
	Curves.Reserve(Count * 3);
	for (int32 i = 0; i < Count; ++i)
	{
		float Value = Weights[i];
		if (i <= 50)
		{
			Value = FMath::Clamp(Value, 0.f, 1.f);
		}

		Curves.Add(ARKitNames[i], Value);

		TArray<FName> Aliases;
		UMugenMorphComponent::GetMetaHumanCtrlAliases(ARKitNames[i], Aliases);
		for (const FName& Alias : Aliases)
		{
			Curves.Add(Alias, Value);
		}
	}

	FBlendedHeapCurve OurCurve;
	UE::Anim::FCurveUtils::BuildUnsorted(OurCurve, Curves);
	// Override so MetaHuman Control Rig idle curves do not win over MugenMorph.
	Output.Curve.Combine(OurCurve);
}

void FAnimNode_MugenMorphFaceSync::GatherDebugData(FNodeDebugData& DebugData)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(GatherDebugData)
	DebugData.AddDebugItem(DebugData.GetNodeName(this));
	SourcePose.GatherDebugData(DebugData);
}
