// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "AnimGraphNode_Base.h"
#include "Animation/AnimNode_MugenMorphFaceSync.h"
#include "AnimGraphNode_MugenMorphFaceSync.generated.h"

UCLASS()
class UAnimGraphNode_MugenMorphFaceSync : public UAnimGraphNode_Base
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_MugenMorphFaceSync Node;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetMenuCategory() const override;
	virtual FString GetNodeCategory() const override;
};
