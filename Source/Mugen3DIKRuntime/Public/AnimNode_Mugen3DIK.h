// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "BoneControllers/AnimNode_ModifyBone.h"
#include "AnimNode_Mugen3DIK.generated.h"

USTRUCT(BlueprintType)
struct FMugen3DIKModifyData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Translations, meta = (PinShownByDefault))
	TArray<FVector> Translations;
};
/**
 * 
 */
USTRUCT(BlueprintInternalUseOnly)
struct MUGEN3DIKRUNTIME_API FAnimNode_Mugen3DIK : public FAnimNode_SkeletalControlBase
{
	GENERATED_USTRUCT_BODY()

	/** Name of bone to control. This is the main bone chain to modify from. **/
	UPROPERTY(EditAnywhere, Category = SkeletalControl)
		FBoneReference BoneToModify;

	/** New translation of bone to apply. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Translation, meta = (PinShownByDefault))
		FVector Translation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Translations, meta = (PinShownByDefault))
		FMugen3DIKModifyData ModifyData;

	/** New rotation of bone to apply. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation, meta = (PinShownByDefault))
		FRotator Rotation;

	/** New rotation of bone to apply. */
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotations, meta = (PinShownByDefault))
		TArray<FRotator> Rotations;*/

	/** New Scale of bone to apply. This is only worldspace. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Scale, meta = (PinShownByDefault))
		FVector Scale;

	/** Whether and how to modify the translation of this bone. */
	UPROPERTY(EditAnywhere, Category = Translation)
		TEnumAsByte<EBoneModificationMode> TranslationMode;

	/** Whether and how to modify the translation of this bone. */
	UPROPERTY(EditAnywhere, Category = Rotation)
		TEnumAsByte<EBoneModificationMode> RotationMode;

	/** Whether and how to modify the translation of this bone. */
	UPROPERTY(EditAnywhere, Category = Scale)
		TEnumAsByte<EBoneModificationMode> ScaleMode;

	/** Reference frame to apply Translation in. */
	UPROPERTY(EditAnywhere, Category = Translation)
		TEnumAsByte<enum EBoneControlSpace> TranslationSpace;

	/** Reference frame to apply Rotation in. */
	UPROPERTY(EditAnywhere, Category = Rotation)
		TEnumAsByte<enum EBoneControlSpace> RotationSpace;

	/** Reference frame to apply Scale in. */
	UPROPERTY(EditAnywhere, Category = Scale)
		TEnumAsByte<enum EBoneControlSpace> ScaleSpace;

	FAnimNode_Mugen3DIK();
	// FAnimNode_Base interface
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// End of FAnimNode_Base interface

	// FAnimNode_SkeletalControlBase interface
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase interface

private:
	// FAnimNode_SkeletalControlBase interface
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase interface
};
