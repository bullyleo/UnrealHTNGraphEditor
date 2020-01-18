// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AIGraphNode.h"
#include "HTNGraphEditorGraphNode.generated.h"

class UEdGraph;
class UEdGraphSchema;

//--------------------------------------------------------------
// グラフに表示されるノードの基底クラス。
// グラフ上での振る舞いにのみ関連するクラスでありHTNの動作そのものには関わらないことに注意。
// 使用するアイコンやツールチップに表示されるテキスト、インプット＆アウトプットピンの作成等が行われる
//--------------------------------------------------------------
UCLASS()
class UHTNGraphEditorGraphNode : public UAIGraphNode
{
	GENERATED_BODY()
public:
	UHTNGraphEditorGraphNode(const FObjectInitializer& ObjectInitializer);

	//~ Begin UEdGraphNode Interface
	virtual class UHTNGraphEditorGraph* GetHTNGraphEditorGraph();
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* DesiredSchema) const override;
	//~ End UEdGraphNode Interface

	virtual void InitializeInstance() override;

	/** gets icon resource name for title bar */
	virtual FName GetNameIcon() const;

	/** if set, this node is root of tree or sub node of it */
	uint32 bRootLevel : 1;

	/** highlight other child node indexes when hovering over a child */
	uint32 bHighlightChildNodeIndices : 1;

	FString DefaultNodeName;
};
