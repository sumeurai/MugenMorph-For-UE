// Copyright 2026 Mugen3D. All Rights Reserved.


#include "AnimGraphNode_Mugen3DMugenMorph.h"

//#include "EdGraph/EdGraphSchema.h"

#define LOCTEXT_NAMESPACE "A3Nodes"

FText UAnimGraphNode_Mugen3DMugenMorph::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeTitle", "Mugen3D MugenMorph Pose");
}

FText UAnimGraphNode_Mugen3DMugenMorph::GetTooltipText() const
{
	return LOCTEXT("NodeTooltip", "Retrieves the current pose associated with the supplied subject");
}

FText UAnimGraphNode_Mugen3DMugenMorph::GetMenuCategory() const
{
	return LOCTEXT("NodeCategory", "Audio To Face");
}

#undef LOCTEXT_NAMESPACE