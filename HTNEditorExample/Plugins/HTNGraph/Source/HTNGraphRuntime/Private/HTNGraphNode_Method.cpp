// Fill out your copyright notice in the Description page of Project Settings.


#include "HTNGraphNode_Method.h"
#include "HTNDomain.h"
#include "HTNBuilder.h"

UHTNGraphNode_Method::UHTNGraphNode_Method(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    NodeName = "Method";

}

void UHTNGraphNode_Method::ConstructMethodBuilder(FHTNBuilder_Domain& DomainBuilder, FHTNBuilder_CompositeTask& CompositeTaskBuilder)
{
    TArray<FHTNCondition> Methods;
    for (const auto& Condition : MethodConditions)
    {
        Methods.Add(FHTNCondition(Condition.MethodWorldState.WorldStateKeyValue, Condition.Check).SetRHSAsValue(Condition.Value));
    }

    FHTNBuilder_Method& MethodsBuilder = CompositeTaskBuilder.AddMethod(Methods);

    for (const auto& Child : Children)
    {
        if (Child.ChildComposite)
        {
            MethodsBuilder.AddTask(*Child.ChildComposite->NodeName);
            if(Child.ChildComposite->Children.Num() > 0)
                Child.ChildComposite->ConstructTaskBuilder(DomainBuilder);
        }
        if (Child.ChildTask)
        {
            MethodsBuilder.AddTask(*Child.ChildTask->NodeName);
            Child.ChildTask->ConstructTaskBuilder(DomainBuilder);
        }
    }
}

#if WITH_EDITOR
void UHTNGraphNode_Method::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    //Get the name of the property that was changed  
    FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

    if ((PropertyName == "Check") || (PropertyName == "MethodConditions")) 
    {
        for (auto& Elem : MethodConditions)
        {
            Elem.bCanEditValue = (Elem.Check == EHTNWorldStateCheck::Greater ||
                                  Elem.Check == EHTNWorldStateCheck::GreaterOrEqual || 
                                  Elem.Check == EHTNWorldStateCheck::Less || 
                                  Elem.Check == EHTNWorldStateCheck::LessOrEqual ||
                                  Elem.Check == EHTNWorldStateCheck::Equal);
        }
    }

    // Call the base class version  
    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif