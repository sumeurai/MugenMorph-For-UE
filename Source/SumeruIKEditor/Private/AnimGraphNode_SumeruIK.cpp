// Copyright 2026 SumeruAI. All Rights Reserved.


#include "AnimGraphNode_SumeruIK.h"


#define LOCTEXT_NAMESPACE "A3Nodes"

FText UAnimGraphNode_SumeruIK::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_SumeruIK_Tooltip", "The Transform Bone node alters the transform - i.e. Translation, Rotation, or Scale - of the bone");
}

FText UAnimGraphNode_SumeruIK::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("SumeruIK", "SumeruIK Modify Bones");
}

#undef LOCTEXT_NAMESPACE