// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "AnimGraphNode_Base.h"
#include "Animation/AnimNode_SumeruMugenMorphFaceSync.h"
#include "AnimGraphNode_SumeruMugenMorphFaceSync.generated.h"

UCLASS()
class UAnimGraphNode_SumeruMugenMorphFaceSync : public UAnimGraphNode_Base
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_SumeruMugenMorphFaceSync Node;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetMenuCategory() const override;
	virtual FString GetNodeCategory() const override;
};
