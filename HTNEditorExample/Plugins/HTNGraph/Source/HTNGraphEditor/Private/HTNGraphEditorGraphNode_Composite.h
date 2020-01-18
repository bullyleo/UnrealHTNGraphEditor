// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "HTNGraphEditorGraphNode.h"
#include "HTNGraphEditorGraphNode_Composite.generated.h"


//--------------------------------------------------------------
// グラフに表示される「Compositeノード」を表すクラス
//--------------------------------------------------------------
UCLASS()
class UHTNGraphEditorGraphNode_Composite : public UHTNGraphEditorGraphNode
{
	GENERATED_BODY()

public:
	UHTNGraphEditorGraphNode_Composite(const FObjectInitializer& ObjectInitializer);

	//virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual bool RefreshNodeClass() override{ return false; }
};
