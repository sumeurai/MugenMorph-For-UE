// Copyright SumeruAI. All Rights Reserved.

#pragma once

#include "AnimNode_SumeruIK.h"
#include "AnimGraphNode_SkeletalControlBase.h"
#include "AnimGraphNode_SumeruIK.generated.h"

/**
 * 
 */

UCLASS(meta = (Keywords = "SumeruIK Modify Transform"))
class SUMERUIKEDITOR_API UAnimGraphNode_SumeruIK : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_SumeruIK Node;

public:
	// UEdGraphNode interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	// End of UEdGraphNode interface
protected:
	virtual const FAnimNode_SkeletalControlBase* GetNode() const override { return &Node; }
};
