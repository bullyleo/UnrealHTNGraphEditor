// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HTNGraphNode.h"
#include "HTNGraphNode_PrimitiveTask.h"
#include "HTNGraphNode_Composite.generated.h"

//--------------------------------------------------------------
// Compositeノードが保持する子ノードデータ
//--------------------------------------------------------------
USTRUCT()
struct FHTNGraphCompositeChild
{
	GENERATED_USTRUCT_BODY()

	/** child node */
	UPROPERTY()
        UHTNGraphNode_Composite* ChildComposite;

	UPROPERTY()
        UHTNGraphNode_PrimitiveTask* ChildTask;
};


//--------------------------------------------------------------
// Compositeノード
// このノードは「Methodノード」のみを子にすることが出来る
//--------------------------------------------------------------
UCLASS()
class HTNGRAPHRUNTIME_API UHTNGraphNode_Composite : public UHTNGraphNode
{
    GENERATED_BODY()
public:
    /** child nodes */
    UPROPERTY()
        TArray<FHTNGraphCompositeChild> Children;

public:
    UHTNGraphNode_Composite(const FObjectInitializer& ObjectInitializer);

    /** fill in data about tree structure */
    void InitializeComposite(uint16 InLastExecutionIndex);

    virtual void ConstructTaskBuilder(struct FHTNBuilder_Domain& DomainBuilder) override;
    virtual void ConstructMethodBuilder(struct FHTNBuilder_Domain& DomainBuilder, struct FHTNBuilder_CompositeTask& CompositeTaskBuilder) {};

    /** @return child node at given index */
    UHTNGraphNode* GetChildNode(int32 Index) const;

//#if WITH_EDITOR
//    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
//#endif

private:
    /** execution index of last node in child branches */
    uint16 LastExecutionIndex;

};
