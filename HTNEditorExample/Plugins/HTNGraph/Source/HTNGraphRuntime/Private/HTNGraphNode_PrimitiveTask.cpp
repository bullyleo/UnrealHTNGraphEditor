// Fill out your copyright notice in the Description page of Project Settings.


#include "HTNGraphNode_PrimitiveTask.h"
#include "HTNBuilder.h"


UHTNGraphNode_PrimitiveTask::UHTNGraphNode_PrimitiveTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Primitive";
}

void UHTNGraphNode_PrimitiveTask::ConstructTaskBuilder(FHTNBuilder_Domain& DomainBuilder)
{
	FHTNBuilder_PrimitiveTask& PrimitiveTaskBuilder = DomainBuilder.AddPrimitiveTask(*NodeName);
	{
		// TODO: èCê≥
		if(TaskOperatorValue != INDEX_NONE)
			PrimitiveTaskBuilder.SetOperator(TaskOperatorValue, OperationParamValue);

		for (const auto& Effect : Effects)
		{
			PrimitiveTaskBuilder.AddEffect(FHTNEffect(Effect.UserDefinitionWorldStateKey.WorldStateKeyValue, Effect.Operation).SetRHSAsValue(Effect.RhsValue));
		}
	}
}

#if WITH_EDITOR
FName UHTNGraphNode_PrimitiveTask::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Task.Icon");
}
#endif
