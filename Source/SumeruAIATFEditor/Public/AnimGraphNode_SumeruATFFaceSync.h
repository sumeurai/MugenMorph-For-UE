// Copyright SumeruAI. All Rights Reserved.

#pragma once

#include "AnimGraphNode_Base.h"
#include "Animation/AnimNode_SumeruATFFaceSync.h"
#include "AnimGraphNode_SumeruATFFaceSync.generated.h"

UCLASS()
class UAnimGraphNode_SumeruATFFaceSync : public UAnimGraphNode_Base
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_SumeruATFFaceSync Node;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetMenuCategory() const override;
	virtual FString GetNodeCategory() const override;
};
