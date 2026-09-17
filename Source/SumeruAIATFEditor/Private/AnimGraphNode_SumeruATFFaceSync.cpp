// Copyright 2026 SumeruAI. All Rights Reserved.

#include "AnimGraphNode_SumeruATFFaceSync.h"

#define LOCTEXT_NAMESPACE "SumeruATFFaceSync"

FText UAnimGraphNode_SumeruATFFaceSync::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeTitle", "SumeruAI ATF Face Sync");
}

FText UAnimGraphNode_SumeruATFFaceSync::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Writes Audio-to-Face ARKit / MetaHuman CTRL curves onto the pose so RigLogic can drive the mouth.");
}

FText UAnimGraphNode_SumeruATFFaceSync::GetMenuCategory() const
{
	return LOCTEXT("MenuCategory", "Audio To Face");
}

FString UAnimGraphNode_SumeruATFFaceSync::GetNodeCategory() const
{
	return TEXT("Audio To Face");
}

#undef LOCTEXT_NAMESPACE
