// Fill out your copyright notice in the Description page of Project Settings.


#include "HTNGraphNode.h"
#include "HTNGraph.h"
#include "HTNGraphNode_Composite.h"

UHTNGraphNode::UHTNGraphNode(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	ExecutionIndex = 0;
}

void UHTNGraphNode::InitializeNode(UHTNGraphNode_Composite* InParentNode, uint16 InExecutionIndex, uint8 InTreeDepth)
{
	ParentNode = InParentNode;
	ExecutionIndex = InExecutionIndex;
	TreeDepth = InTreeDepth;
}

void UHTNGraphNode::InitializeFromAsset(UHTNGraph& Asset)
{
	TreeAsset = &Asset;
}

#include "BehaviorTree/BehaviorTreeTypes.h"
FString UHTNGraphNode::GetNodeName() const
{
	return NodeName.Len() ? NodeName : UBehaviorTreeTypes::GetShortTypeName(this);
}
