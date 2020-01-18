// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HTNGraphEditorGraphNode.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "ToolMenus.h"
#include "HTNGraph.h"
#include "HTNGraphNode.h"
#include "HTNGraphEditorTypes.h"
#include "HTNGraphEditorGraph.h"
#include "EdGraphSchema_HTNGraph.h"
#include "SGraphEditorActionMenuAI.h"
#include "GraphDiffControl.h"

#define LOCTEXT_NAMESPACE "HTNGraphEditor"


UHTNGraphEditorGraphNode::UHTNGraphEditorGraphNode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bRootLevel = false;
	bHighlightChildNodeIndices = false;
	DefaultNodeName = "";
}

void UHTNGraphEditorGraphNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UHTNGraphEditorTypes::PinCategory_MultipleNodes, TEXT("In"));
	CreatePin(EGPD_Output, UHTNGraphEditorTypes::PinCategory_MultipleNodes, TEXT("Out"));
}

FText UHTNGraphEditorGraphNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	// ユーザー定義のノード名がある場合、そちらを使用する。
	const UHTNGraphNode* MyNode = Cast<UHTNGraphNode>(NodeInstance);
	if (MyNode != NULL)
	{
		return FText::FromString(MyNode->GetNodeName());
	}

	return FText::FromString(DefaultNodeName);
}

void UHTNGraphEditorGraphNode::InitializeInstance()
{
	UHTNGraphNode* HTNNode = Cast<UHTNGraphNode>(NodeInstance);
	UHTNGraph* HTNAsset = HTNNode ? Cast<UHTNGraph>(HTNNode->GetOuter()) : nullptr;
	if (HTNNode && HTNAsset)
	{
		HTNNode->InitializeFromAsset(*HTNAsset);
		HTNNode->InitializeNode(NULL, MAX_uint16, 0);
	}
}

FText UHTNGraphEditorGraphNode::GetTooltipText() const
{
	FText TooltipDesc;

	if (TooltipDesc.IsEmpty())
	{
		TooltipDesc = Super::GetTooltipText();
	}

	return TooltipDesc;
}

UHTNGraphEditorGraph* UHTNGraphEditorGraphNode::GetHTNGraphEditorGraph()
{
	return CastChecked<UHTNGraphEditorGraph>(GetGraph());
}

bool UHTNGraphEditorGraphNode::CanCreateUnderSpecifiedSchema(const UEdGraphSchema* DesiredSchema) const
{
	return DesiredSchema->GetClass()->IsChildOf(UEdGraphSchema_HTNGraph::StaticClass());
}

FName UHTNGraphEditorGraphNode::GetNameIcon() const
{
	UHTNGraphNode* HTNGraphNodeInstance = Cast<UHTNGraphNode>(NodeInstance);
	return HTNGraphNodeInstance != nullptr ? HTNGraphNodeInstance->GetNodeIconName() : FName("BTEditor.Graph.BTNode.Icon");
}

#undef LOCTEXT_NAMESPACE
