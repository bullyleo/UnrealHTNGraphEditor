// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "HTNGraphEditorGraphNode.h"
#include "HTNGraphEditorGraphNode_Method.generated.h"

//--------------------------------------------------------------
// グラフに表示される「Compositeノード」を表すクラス
//--------------------------------------------------------------
UCLASS()
class UHTNGraphEditorGraphNode_Method : public UHTNGraphEditorGraphNode
{
	GENERATED_BODY()

public:
	UHTNGraphEditorGraphNode_Method(const FObjectInitializer& ObjectInitializer);

	virtual FText GetTooltipText() const override;
	virtual bool RefreshNodeClass() override { return false; }
};
