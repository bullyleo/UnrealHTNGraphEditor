// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HTNGraphEditorGraphNode_Root.h"
#include "UObject/UObjectIterator.h"
#include "HTNGraphEditorTypes.h"


UHTNGraphEditorGraphNode_Root::UHTNGraphEditorGraphNode_Root(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bIsReadOnly = true;
}

void UHTNGraphEditorGraphNode_Root::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();
}

void UHTNGraphEditorGraphNode_Root::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UHTNGraphEditorTypes::PinCategory_SingleComposite, TEXT("In"));
}

FText UHTNGraphEditorGraphNode_Root::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("HTNGraphEditor", "Root", "ROOT");
}

FName UHTNGraphEditorGraphNode_Root::GetNameIcon() const
{
	return FName("BTEditor.Graph.BTNode.Root.Icon");
}

FText UHTNGraphEditorGraphNode_Root::GetTooltipText() const
{
	return UEdGraphNode::GetTooltipText();
}

void UHTNGraphEditorGraphNode_Root::PostEditUndo()
{
	Super::PostEditUndo();
}

FText UHTNGraphEditorGraphNode_Root::GetDescription() const
{
	return FText::FromString("Root Node Description");
}
