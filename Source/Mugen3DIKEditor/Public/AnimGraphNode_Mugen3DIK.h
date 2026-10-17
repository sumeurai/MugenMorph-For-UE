// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "AnimNode_Mugen3DIK.h"
#include "AnimGraphNode_SkeletalControlBase.h"
#include "AnimGraphNode_Mugen3DIK.generated.h"

/**
 * 
 */

UCLASS(meta = (Keywords = "Mugen3DIK Modify Transform"))
class MUGEN3DIKEDITOR_API UAnimGraphNode_Mugen3DIK : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_Mugen3DIK Node;

public:
	// UEdGraphNode interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	// End of UEdGraphNode interface
protected:
	virtual const FAnimNode_SkeletalControlBase* GetNode() const override { return &Node; }
};
