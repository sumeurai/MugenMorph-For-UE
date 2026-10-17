// Copyright 2026 Mugen3D. All Rights Reserved.

#include "AnimGraphNode_SumeruMugenMorphFaceSync.h"

#define LOCTEXT_NAMESPACE "SumeruMugenMorphFaceSync"

FText UAnimGraphNode_SumeruMugenMorphFaceSync::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeTitle", "Mugen3D MugenMorph Face Sync");
}

FText UAnimGraphNode_SumeruMugenMorphFaceSync::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Writes Audio-to-Face ARKit / MetaHuman CTRL curves onto the pose so RigLogic can drive the mouth.");
}

FText UAnimGraphNode_SumeruMugenMorphFaceSync::GetMenuCategory() const
{
	return LOCTEXT("MenuCategory", "Audio To Face");
}

FString UAnimGraphNode_SumeruMugenMorphFaceSync::GetNodeCategory() const
{
	return TEXT("Audio To Face");
}

#undef LOCTEXT_NAMESPACE
