// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HTNGraphEditorGraphNode_Composite.h"
#include "HTNGraphNode.h"

UHTNGraphEditorGraphNode_Composite::UHTNGraphEditorGraphNode_Composite(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultNodeName = "Composite Node";
}

//FText UHTNGraphEditorGraphNode_Composite::GetNodeTitle(ENodeTitleType::Type TitleType) const
//{
//	// ユーザー定義のノード名がある場合、そちらを使用する。
//	const UHTNGraphNode* MyNode = Cast<UHTNGraphNode>(NodeInstance);
//	if (MyNode != NULL)
//	{
//		return FText::FromString(MyNode->GetNodeName());
//	}
//
//	return FText::FromString(DefaultNodeName);
//}


FText UHTNGraphEditorGraphNode_Composite::GetTooltipText() const
{
	const UHTNGraphNode_Composite* CompositeNode = Cast<UHTNGraphNode_Composite>(NodeInstance);
	if (CompositeNode)
	{
		return FText::Format(FText::FromString(TEXT("{0}\n\n{1}")), 
			Super::GetDescription(), 
			NSLOCTEXT("HTNGraphEditor", "CompositeNodeScopeTooltip", "This node is a local scope for decorators.\nAll observing decorators (Lower Priority or Both) will be removed when execution flow leaves this branch."));
	}

	return Super::GetTooltipText();
}
