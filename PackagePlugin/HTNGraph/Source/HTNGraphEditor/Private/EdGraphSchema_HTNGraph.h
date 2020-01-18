// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphSchema.h"
#include "AIGraphTypes.h"
#include "AIGraphSchema.h"
#include "EdGraphSchema_HTNGraph.generated.h"

class FSlateRect;
class UEdGraph;

// Auto Arrange（自動整列）アクション
USTRUCT()
struct FHTNGraphSchemaAction_AutoArrange : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	FHTNGraphSchemaAction_AutoArrange()
		: FEdGraphSchemaAction() {}

	FHTNGraphSchemaAction_AutoArrange(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
	{}

	//~ Begin FEdGraphSchemaAction Interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	//~ End FEdGraphSchemaAction Interface
};


/**
* エディタ上で右クリックしたときに表示されるコンテキストメニューの振る舞いやノード間の接続を定義する
*/
UCLASS(MinimalAPI)
class UEdGraphSchema_HTNGraph : public UAIGraphSchema
{
	GENERATED_BODY()

public:
	UEdGraphSchema_HTNGraph(const FObjectInitializer& ObjectInitializer);

	//~ Begin EdGraphSchema Interface

	// 新しくグラフが生成された時の既定ノードを設定する
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	// グラフを右クリックした時やピンをドラッグ＆リリースしたときに実行できるすべてのアクションを取得する
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	// ピン間の接続可否を設定する
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
	// FConnectionDrawingPolicy のインスタンスを作成する
	virtual class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const override;


	virtual bool IsCacheVisualizationOutOfDate(int32 InVisualizationCacheID) const override
	{
		return CurrentCacheRefreshID != InVisualizationCacheID;
	}
	virtual int32 GetCurrentVisualizationCacheID() const override
	{
		return CurrentCacheRefreshID;
	}
	virtual void ForceVisualizationCacheClear() const override
	{
		++CurrentCacheRefreshID;
	}
	//~ End EdGraphSchema Interface

private:
	// コンテキストメニューでのアクションデータを構築する
	void ConstructGraphContextActionData(UClass* GatherClass, UClass* TargetClass, UClass* GenerateNodeClass, FString CategoryName, FGraphNodeClassHelper* ClassCache, FGraphContextMenuBuilder& ContextMenuBuilder) const;

private:
	// ID for checking dirty status of node titles against, increases whenever 
	static int32 CurrentCacheRefreshID;
};
