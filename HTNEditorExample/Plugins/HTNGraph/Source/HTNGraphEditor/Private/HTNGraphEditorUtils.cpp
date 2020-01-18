// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HTNGraphEditorUtils.h"
#include "EdGraph/EdGraph.h"
#include "HTNGraphEditorGraphNode_Composite.h"
#include "HTNGraphEditorGraphNode_PrimitiveTask.h"
#include "HTNGraphEditorGraphNode_Method.h"

TArray<UObject*> HTNGraphEditorUtils::GetSelectionForPropertyEditor(const TSet<UObject*>& InSelection, FPropertySelectionInfo& OutSelectionInfo)
{
	TArray<UObject*> Selection;

	for (UObject* SelectionEntry : InSelection)
	{
		UHTNGraphEditorGraphNode_Method* GraphNode_Method = Cast<UHTNGraphEditorGraphNode_Method>(SelectionEntry);
		if (GraphNode_Method)
		{
			Selection.Add(GraphNode_Method->NodeInstance);
			continue;
		}

		UHTNGraphEditorGraphNode_Composite* GraphNode_Composite = Cast<UHTNGraphEditorGraphNode_Composite>(SelectionEntry);
		if (GraphNode_Composite)
		{
			Selection.Add(GraphNode_Composite->NodeInstance);
			continue;
		}

		UHTNGraphEditorGraphNode_PrimitiveTask* GraphNode_Task = Cast<UHTNGraphEditorGraphNode_PrimitiveTask>(SelectionEntry);
		if (GraphNode_Task)
		{
			Selection.Add(GraphNode_Task->NodeInstance);
			continue;
		}

		Selection.Add(SelectionEntry);
	}

	return Selection;
}
