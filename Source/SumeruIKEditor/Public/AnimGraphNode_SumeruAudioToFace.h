// Copyright 2026 SumeruAI. All Rights Reserved.

#pragma once

#include "AnimGraphNode_Base.h"
#include "AnimNode_SumeruAIAudioToFace.h"

#include "AnimGraphNode_SumeruAudioToFace.generated.h"



UCLASS()
class UAnimGraphNode_SumeruAIAudioToFace : public UAnimGraphNode_Base
{
	GENERATED_BODY()

public:

	//~ UEdGraphNode interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetMenuCategory() const;
	//~ End of UEdGraphNode

	//~ Begin UK2Node interface
	//virtual void ConvertDeprecatedNode(UEdGraph* Graph, bool bOnlySafeChanges) override;
	//~ End UK2Node interface

public:

	UPROPERTY(EditAnywhere, Category = Settings)
		FAnimNode_SumeruAIAudioToFace Node;

};
