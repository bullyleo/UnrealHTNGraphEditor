// Fill out your copyright notice in the Description page of Project Settings.


#include "HTNGraphNode_Composite.h"
#include "HTNDomain.h"
#include "HTNBuilder.h"

UHTNGraphNode_Composite::UHTNGraphNode_Composite(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Composite";
}

void UHTNGraphNode_Composite::InitializeComposite(uint16 InLastExecutionIndex)
{
	LastExecutionIndex = InLastExecutionIndex;
}

void UHTNGraphNode_Composite::ConstructTaskBuilder(FHTNBuilder_Domain& DomainBuilder)
{
	FHTNBuilder_CompositeTask& CompositeTaskBuilder = DomainBuilder.AddCompositeTask(*NodeName);
	for (const auto& Child : Children)
	{
		Child.ChildComposite->ConstructMethodBuilder(DomainBuilder, CompositeTaskBuilder);
	}
}

UHTNGraphNode* UHTNGraphNode_Composite::GetChildNode(int32 Index) const
{
	if (Children.IsValidIndex(Index))
	{
		if (Children[Index].ChildComposite)
			return (UHTNGraphNode*)Children[Index].ChildComposite;
		else
			return (UHTNGraphNode*)Children[Index].ChildTask;
	}

	return nullptr;
}
