// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HTNGraphEditorGraphNode_Method.h"
#include "HTNGraphEditorTypes.h"
#include "HTNGraphNode.h"


UHTNGraphEditorGraphNode_Method::UHTNGraphEditorGraphNode_Method(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DefaultNodeName = "Method Node";
}

FText UHTNGraphEditorGraphNode_Method::GetTooltipText() const
{
	//return FText::Format(FText::FromString("Any tooltip"));

	return Super::GetTooltipText();
}

