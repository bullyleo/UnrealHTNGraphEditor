// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HTNGraphEditorGraphNode_Composite.h"
#include "HTNGraphNode.h"

UHTNGraphEditorGraphNode_Composite::UHTNGraphEditorGraphNode_Composite(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultNodeName = "Composite Node";
}


FText UHTNGraphEditorGraphNode_Composite::GetTooltipText() const
{
	//return FText::Format(FText::FromString("Any tooltip"));

	return Super::GetTooltipText();
}
