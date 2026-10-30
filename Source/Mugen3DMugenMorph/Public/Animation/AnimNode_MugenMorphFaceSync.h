// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNodeBase.h"
#include "MugenMorphComponent.h"
#include "AnimNode_MugenMorphFaceSync.generated.h"

/**
 * Convai-style face node: evaluate source pose, then write ARKit + MetaHuman
 * CTRL_expressions_* curves so RigLogic in the Face AnimBP can drive the mouth.
 */
USTRUCT(BlueprintInternalUseOnly)
struct MUGEN3DMUGENMORPH_API FAnimNode_MugenMorphFaceSync : public FAnimNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Links")
	FPoseLink SourcePose;

	/** Optional. If unset, the first UMugenMorphComponent on the owning actor is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph", meta = (PinHiddenByDefault))
	TObjectPtr<UMugenMorphComponent> MugenMorphComponent;

	FAnimNode_MugenMorphFaceSync();

	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	virtual bool HasPreUpdate() const override { return true; }
	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;

private:
	TWeakObjectPtr<UMugenMorphComponent> ResolvedComponent;
};
