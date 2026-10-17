// Copyright 2026 Mugen3D. All Rights Reserved.


#include "AnimGraphNode_Mugen3DIK.h"


#define LOCTEXT_NAMESPACE "A3Nodes"

FText UAnimGraphNode_Mugen3DIK::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_Mugen3DIK_Tooltip", "The Transform Bone node alters the transform - i.e. Translation, Rotation, or Scale - of the bone");
}

FText UAnimGraphNode_Mugen3DIK::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Mugen3DIK", "Mugen3DIK Modify Bones");
}

#undef LOCTEXT_NAMESPACE