// Copyright 2026 SumeruAI. All Rights Reserved.


#include "AnimGraphNode_SumeruAudioToFace.h"

//#include "EdGraph/EdGraphSchema.h"

#define LOCTEXT_NAMESPACE "A3Nodes"

FText UAnimGraphNode_SumeruAIAudioToFace::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeTitle", "SumeruAI ATF Pose");
}

FText UAnimGraphNode_SumeruAIAudioToFace::GetTooltipText() const
{
	return LOCTEXT("NodeTooltip", "Retrieves the current pose associated with the supplied subject");
}

FText UAnimGraphNode_SumeruAIAudioToFace::GetMenuCategory() const
{
	return LOCTEXT("NodeCategory", "Audio To Face");
}

#undef LOCTEXT_NAMESPACE