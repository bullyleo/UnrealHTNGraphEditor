// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HTNGraphNode.h"
#include "HTNGraphNode_PrimitiveTask.h"
#include "HTNGraphNode_Composite.h"
#include "HTNPlanner.h" // HTN Planner plugin
#include "HTNGraphNode_Method.generated.h"

//--------------------------------------------------------------
// Methodの子ノードを展開するための事前条件データ
//--------------------------------------------------------------
USTRUCT()
struct FHTNMethodCondition
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, Category = "HTN")
        FUserDefinitionWorldState MethodWorldState;

    UPROPERTY(EditAnywhere, Category = "AI|HTN")
        EHTNWorldStateCheck Check;

    // 詳細パネルの項目編集可能フラグ
    UPROPERTY(/*EditAnywhere, Category = "AI|HTN"*/)
        bool bCanEditValue;

    // Editable only if bCanEditValue is True.
    UPROPERTY(EditAnywhere, meta = (EditCondition = "bCanEditValue"), Category = "AI|HTN" )
        int32 Value;
};


//--------------------------------------------------------------
// いくつかのノードをまとめる.
// MethodConditionsの結果によって子ノードを展開するか決定する.
// 「Compositeノード」「Primitive Taskノード」のみを子にすることが出来る 
//--------------------------------------------------------------
UCLASS()
class HTNGRAPHRUNTIME_API UHTNGraphNode_Method : public UHTNGraphNode_Composite
{
	GENERATED_BODY()
public:

    UPROPERTY(EditAnywhere, Category = "HTN")
        TArray<FHTNMethodCondition> MethodConditions;

public:
    UHTNGraphNode_Method(const FObjectInitializer& ObjectInitializer);

    virtual void ConstructMethodBuilder(struct FHTNBuilder_Domain& DomainBuilder, struct FHTNBuilder_CompositeTask& CompositeTaskBuilder) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

};
