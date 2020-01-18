// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "HTNGraphEditorGraphNode.h"
#include "Engine/UserDefinedEnum.h"
#include "Engine/DataTable.h"
#include "HTNGraphEditorGraphNode_Root.generated.h"

/** Root node of this behavior tree, holds Blackboard data */
//--------------------------------------------------------------
// グラフに表示される「ROOTノード」を表すクラス
//--------------------------------------------------------------
UCLASS()
class UHTNGraphEditorGraphNode_Root : public UHTNGraphEditorGraphNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "HTN")
		UEnum* WorldStateEnumAsset;
	UPROPERTY(EditAnywhere, Category = "HTN")
		UEnum* OperatorEnumAsset;
	UPROPERTY(EditAnywhere, Category = "HTN")
		UEnum* OperatorParamEnumAsset;

	UHTNGraphEditorGraphNode_Root(const FObjectInitializer& ObjectInitializer);

	virtual void PostPlacedNewNode() override;
	virtual void AllocateDefaultPins() override;
	virtual bool CanDuplicateNode() const override { return false; }
	virtual bool CanUserDeleteNode() const override{ return false; }
	virtual bool HasErrors() const override { return false; }
	virtual bool RefreshNodeClass() override { return false; }
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	/** gets icon resource name for title bar */
	virtual FName GetNameIcon() const override;
	virtual FText GetTooltipText() const override;

	virtual void PostEditUndo() override;
	virtual FText GetDescription() const override;
};
