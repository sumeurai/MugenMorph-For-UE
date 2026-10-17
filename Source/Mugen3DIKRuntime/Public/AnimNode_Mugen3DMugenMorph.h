// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once
#include "Animation/AnimNodeBase.h"
#include "CoreMinimal.h"
#include "LiveLinkClientReference.h"
#include "LiveLinkRetargetAsset.h"
#include "LiveLinkTypes.h"
#include "Curves/CurveFloat.h"
#include "../../Mugen3DMugenMorph/Public/MugenMorph_Type.h"
#include "AnimNode_Mugen3DMugenMorph.generated.h"
/**
 * Optional AnimGraph helper for MugenMorph.
 * Preferred runtime path for Talk / external lip-sync is:
 *   UTalkStreamMugenMorphBridgeComponent -> AMugenMorphManager -> UMugenMorphComponent (SetMorphTarget).
 * Keep this node only when an AnimBP must sample LiveLink / EmoteData pins directly.
 */
class ILiveLinkClient;
class UMugenMorphAnim;

PRAGMA_DISABLE_DEPRECATION_WARNINGS

USTRUCT(BlueprintInternalUseOnly)
struct MUGEN3DIKRUNTIME_API FAnimNode_Mugen3DMugenMorph : public FAnimNode_Base
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
		FPoseLink InputPose;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SourceData, meta = (PinShownByDefault))
		FAudioExpressionData CustomExpression;*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SourceData)
		int FrameCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SourceData)
		float PlayInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SourceData, meta = (PinShownByDefault))
		int Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SourceData, meta = (PinShownByDefault))
	    FAudioExpression EmoteData;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SourceData, meta = (PinShownByDefault))
	// 	UCurveFloat* BsCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SourceData, meta = (PinShownByDefault))
		FLiveLinkSubjectName LiveLinkSubjectName;

#if WITH_EDITORONLY_DATA
	UE_DEPRECATED(4.23, "FName SubjectName is deprecated. Use the SubjectName of type FLiveLinkSubjectName instead.")
		UPROPERTY()
		FName SubjectName_DEPRECATED;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear, Category = Retarget, meta = (NeverAsPin))
		TSubclassOf<ULiveLinkRetargetAsset> RetargetAsset;

	UPROPERTY(transient)
		ULiveLinkRetargetAsset* CurrentRetargetAsset;

public:
	FAnimNode_Mugen3DMugenMorph();

	//~ FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual bool HasPreUpdate() const { return true; }
	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	//~ End of FAnimNode_Base interface

	bool Serialize(FArchive& Ar);

protected:
	virtual void OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance) override;

private:

	FLiveLinkClientReference LiveLinkClient_GameThread;
	ILiveLinkClient* LiveLinkClient_AnyThread;
	const UAnimInstance* CurAnimInstance;
	FAudioExpressionData CustomExpression;
	bool StartEmote;
	// Delta time from update so that it can be passed to retargeter
	float CachedDeltaTime;
	TArray<float> CurFrame;
	float Weight;
	int Direction;
};

PRAGMA_ENABLE_DEPRECATION_WARNINGS
