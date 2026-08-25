// Copyright SumeruAI. All Rights Reserved.

#include "Animation/AnimNode_SumeruATFFaceSync.h"

#include "ATFComponent.h"
#include "Animation/AnimCurveUtils.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

FAnimNode_SumeruATFFaceSync::FAnimNode_SumeruATFFaceSync()
{
}

void FAnimNode_SumeruATFFaceSync::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Initialize_AnyThread)
	SourcePose.Initialize(Context);
	ResolvedComponent = nullptr;
}

void FAnimNode_SumeruATFFaceSync::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(CacheBones_AnyThread)
	SourcePose.CacheBones(Context);
}

void FAnimNode_SumeruATFFaceSync::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Update_AnyThread)
	SourcePose.Update(Context);
}

void FAnimNode_SumeruATFFaceSync::PreUpdate(const UAnimInstance* InAnimInstance)
{
	if (ATFComponent && IsValid(ATFComponent))
	{
		ResolvedComponent = ATFComponent;
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

	UATFComponent* Found = Owner->FindComponentByClass<UATFComponent>();
	if (!Found)
	{
		if (AActor* Parent = Owner->GetAttachParentActor())
		{
			Found = Parent->FindComponentByClass<UATFComponent>();
		}
	}
	ResolvedComponent = Found;
}

void FAnimNode_SumeruATFFaceSync::Evaluate_AnyThread(FPoseContext& Output)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Evaluate_AnyThread)
	SourcePose.Evaluate(Output);

	UATFComponent* Component = ResolvedComponent.Get();
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

	const TArray<FName>& ARKitNames = UATFComponent::GetARKitNames();
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
		UATFComponent::GetMetaHumanCtrlAliases(ARKitNames[i], Aliases);
		for (const FName& Alias : Aliases)
		{
			Curves.Add(Alias, Value);
		}
	}

	FBlendedHeapCurve OurCurve;
	UE::Anim::FCurveUtils::BuildUnsorted(OurCurve, Curves);
	// Override so MetaHuman Control Rig idle curves do not win over ATF.
	Output.Curve.Combine(OurCurve);
}

void FAnimNode_SumeruATFFaceSync::GatherDebugData(FNodeDebugData& DebugData)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(GatherDebugData)
	DebugData.AddDebugItem(DebugData.GetNodeName(this));
	SourcePose.GatherDebugData(DebugData);
}
