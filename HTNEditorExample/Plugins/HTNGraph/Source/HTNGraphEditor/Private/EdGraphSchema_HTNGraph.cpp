// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "EdGraphSchema_HTNGraph.h"
#include "HTNGraphEditorModule.h"
#include "HTNGraphEditorTypes.h"
#include "HTNGraphEditorGraph.h"
#include "HTNGraphEditorGraphNode.h"
#include "HTNGraphEditorGraphNode_Composite.h"
#include "HTNGraphEditorGraphNode_Root.h"
#include "HTNGraphEditorGraphNode_PrimitiveTask.h"
#include "HTNGraphEditorGraphNode_Method.h"
#include "HTNGraph.h"
#include "HTNGraphNode_PrimitiveTask.h"
#include "HTNGraphNode_Method.h"
#include "HTNGraphConnectionDrawingPolicy.h"

#include "EdGraph/EdGraph.h"
#include "Layout/SlateRect.h"
//#include "EdGraphNode_Comment.h"
#include "Modules/ModuleManager.h"
//#include "Framework/MultiBox/MultiBoxBuilder.h"
//#include "ToolMenus.h"
//#include "GraphEditorActions.h"
//#include "Toolkits/ToolkitManager.h"

#define LOCTEXT_NAMESPACE "HTNGraphEditor"

int32 UEdGraphSchema_HTNGraph::CurrentCacheRefreshID = 0;

//----------------------------------------------------------------------//
// FHTNGraphSchemaAction_AutoArrange
//----------------------------------------------------------------------//
UEdGraphNode* FHTNGraphSchemaAction_AutoArrange::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UHTNGraphEditorGraph* Graph = Cast<UHTNGraphEditorGraph>(ParentGraph);
	if (Graph) Graph->AutoArrange();
	return NULL;
}


//----------------------------------------------------------------------//
// UEdGraphSchema_HTNGraph
//----------------------------------------------------------------------//

UEdGraphSchema_HTNGraph::UEdGraphSchema_HTNGraph(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{}

void UEdGraphSchema_HTNGraph::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	FGraphNodeCreator<UHTNGraphEditorGraphNode_Root> NodeCreator(Graph);
	UHTNGraphEditorGraphNode_Root* MyNode = NodeCreator.CreateNode();
	NodeCreator.Finalize();
	SetNodeMetaData(MyNode, FNodeMetadata::DefaultGraphNode);
}


void UEdGraphSchema_HTNGraph::ConstructGraphContextActionData(UClass* GatherClass, UClass* TargetClass, UClass* GenerateNodeClass, FString CategoryName, FGraphNodeClassHelper* ClassCache, FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	// コンテキストメニューのカテゴリ名を指定
	FCategorizedGraphActionListBuilder CompositesBuilder(CategoryName);

	// UHTNGraphNode_Compositeもしくはそれを継承するクラスをすべて取得
	TArray<FGraphNodeClassData> NodeClasses;
	ClassCache->GatherClasses(GatherClass, NodeClasses);

	for (const auto& NodeClass : NodeClasses)
	{
		// 対象クラスが指定されている かつ NodeClassのクラス名と対象クラスのクラス名と異なる場合は以降の処理をスキップする
		if (TargetClass && NodeClass.GetClassName() != TargetClass->GetName())
			continue;

		const FText NodeTypeName = FText::FromString(FName::NameToDisplayString(NodeClass.ToString(), false));
		// コンテキストメニューにアクションを追加
		TSharedPtr<FAISchemaAction_NewNode> AddOpAction = UAIGraphSchema::AddNewNodeAction(CompositesBuilder, NodeClass.GetCategory(), NodeTypeName, FText::GetEmpty());
		// アクションが選択されたときに生成されるノードを指定
		UClass* GraphNodeClass = GenerateNodeClass;
		// ノードインスタンスを生成
		UHTNGraphEditorGraphNode* OpNode = NewObject<UHTNGraphEditorGraphNode>(ContextMenuBuilder.OwnerOfTemporaries, GraphNodeClass);
		OpNode->ClassData = NodeClass;
		AddOpAction->NodeTemplate = OpNode;
	}
	// これまでに指定したCompositesBuilderをコンテキストメニューに追加する
	ContextMenuBuilder.Append(CompositesBuilder);
}


void UEdGraphSchema_HTNGraph::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	// ピンを引っ張っている場合に表示するコンテキストメニュー
	const bool bNoParent = (ContextMenuBuilder.FromPin == NULL);

	FHTNGraphEditorModule& EditorModule = FModuleManager::GetModuleChecked<FHTNGraphEditorModule>(TEXT("HTNGraphEditor"));
	FGraphNodeClassHelper* ClassCache = EditorModule.GetClassCache().Get();

	// ピンを引っ張っている場合
	if (bNoParent == false)
	{
		// ピンを引っ張ってきたノードを取得
		const UEdGraphNode* FromNode = ContextMenuBuilder.FromPin->GetOwningNode();
		// そのノードの型が「UHTNGraphEditorGraphNode_Root」である場合。
		// Rootノードからは「Compositeノードのみ」生成できる。
		if (FromNode->IsA<UHTNGraphEditorGraphNode_Root>())
		{
			ConstructGraphContextActionData(
				UHTNGraphNode_Composite::StaticClass(),
				UHTNGraphNode_Composite::StaticClass(),
				UHTNGraphEditorGraphNode_Composite::StaticClass(),
				"Composited", ClassCache, ContextMenuBuilder
			);
		}
		// そのノードの型が「UHTNGraphEditorGraphNode_Composite」である場合。
		// Compositeノードは「Methodノードのみ」生成できる
		else if (FromNode->IsA<UHTNGraphEditorGraphNode_Composite>())
		{
			ConstructGraphContextActionData(
				UHTNGraphNode_Method::StaticClass(),
				nullptr,
				UHTNGraphEditorGraphNode_Method::StaticClass(),
				"Composited", ClassCache, ContextMenuBuilder
			);
		}
		// そのノードの型が「UHTNGraphEditorGraphNode_Method」である場合。
		// Methodノードからは「CompositeノードとPrimitiveTaskノードのみ」生成できる。
		else if (FromNode->IsA<UHTNGraphEditorGraphNode_Method>())
		{
			ConstructGraphContextActionData(
				UHTNGraphNode_Composite::StaticClass(),
				UHTNGraphNode_Composite::StaticClass(),
				UHTNGraphEditorGraphNode_Composite::StaticClass(),
				"Composited", ClassCache, ContextMenuBuilder
			);

			ConstructGraphContextActionData(
				UHTNGraphNode_PrimitiveTask::StaticClass(),
				nullptr,
				UHTNGraphEditorGraphNode_PrimitiveTask::StaticClass(),
				"Tasks", ClassCache, ContextMenuBuilder
			);
		}
	}

	// ピンを引き出さない or グラフエディタ上で右クリックしたときに表示されるコンテキストメニューの内容
	if (bNoParent)
	{
		ConstructGraphContextActionData(
			UHTNGraphNode_Composite::StaticClass(),
			UHTNGraphNode_Composite::StaticClass(),
			UHTNGraphEditorGraphNode_Composite::StaticClass(),
			"Composited", ClassCache, ContextMenuBuilder
		);

		ConstructGraphContextActionData(
			UHTNGraphNode_Method::StaticClass(),
			nullptr,
			UHTNGraphEditorGraphNode_Method::StaticClass(),
			"Composited", ClassCache, ContextMenuBuilder
		);

		ConstructGraphContextActionData(
			UHTNGraphNode_PrimitiveTask::StaticClass(),
			nullptr,
			UHTNGraphEditorGraphNode_PrimitiveTask::StaticClass(),
			"Tasks", ClassCache, ContextMenuBuilder
		);

		// グラフエディタ右クリック時に表示されるコンテキストメニューの内容を定義（Auto Arrange）
		TSharedPtr<FHTNGraphSchemaAction_AutoArrange> Action = TSharedPtr<FHTNGraphSchemaAction_AutoArrange>(
			new FHTNGraphSchemaAction_AutoArrange(FText::GetEmpty(), LOCTEXT("AutoArrange", "Auto Arrange"), FText::GetEmpty(), 0)
			);

		ContextMenuBuilder.AddAction(Action);
	}

}


const FPinConnectionResponse UEdGraphSchema_HTNGraph::CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const
{
	// Make sure the pins are not on the same node
	if (PinA->GetOwningNode() == PinB->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorSameNode", "Both are on the same node"));
	}


	const bool bPinAIsTask = PinA->GetOwningNode()->IsA(UHTNGraphEditorGraphNode_PrimitiveTask::StaticClass());
	const bool bPinAIsComposite = PinA->GetOwningNode()->IsA(UHTNGraphEditorGraphNode_Composite::StaticClass());
	const bool bPinAIsMethod = PinA->GetOwningNode()->IsA(UHTNGraphEditorGraphNode_Method::StaticClass());
	const bool bPinAIsRoot = PinA->GetOwningNode()->IsA(UHTNGraphEditorGraphNode_Root::StaticClass());

	const bool bPinBIsTask = PinB->GetOwningNode()->IsA(UHTNGraphEditorGraphNode_PrimitiveTask::StaticClass());
	const bool bPinBIsComposite = PinB->GetOwningNode()->IsA(UHTNGraphEditorGraphNode_Composite::StaticClass());
	const bool bPinBIsMethod = PinB->GetOwningNode()->IsA(UHTNGraphEditorGraphNode_Method::StaticClass());
	const bool bPinBIsRoot = PinB->GetOwningNode()->IsA(UHTNGraphEditorGraphNode_Root::StaticClass());

	// Deny : Method -> Method
	if ((bPinAIsMethod && bPinBIsMethod))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorOnlyComposite", "Composite node and Primitive node are allowed"));
	}
	// Deny : Composite -> Composite or Composite -> Primitive
	if ((bPinAIsComposite && bPinBIsComposite) || (bPinAIsComposite && bPinBIsTask))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorOnlyComposite", "Only Method nodes are allowed"));
	}
	// Deny : ROOT -> Primitive or ROOT -> Method
	if ((bPinAIsRoot && bPinBIsTask) || (bPinAIsRoot && bPinBIsMethod))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorOnlyComposite", "Only Composite nodes are allowed"));
	}

	// 入力ピン同士は接続できない
	if ((PinA->Direction == EGPD_Input) && (PinB->Direction == EGPD_Input))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorInput", "Can't connect input node to input node"));
	}
	// 出力ピン同士も接続できない
	else if ((PinB->Direction == EGPD_Output) && (PinA->Direction == EGPD_Output))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorOutput", "Can't connect output node to output node"));
	}

	class FNodeVisitorCycleChecker
	{
	public:
		/** Check whether a loop in the graph would be caused by linking the passed-in nodes */
		bool CheckForLoop(UEdGraphNode* StartNode, UEdGraphNode* EndNode)
		{
			VisitedNodes.Add(EndNode);
			return TraverseInputNodesToRoot(StartNode);
		}

	private:
		/**
		 * Helper function for CheckForLoop()
		 * @param	Node	The node to start traversal at
		 * @return true if we reached a root node (i.e. a node with no input pins), false if we encounter a node we have already seen
		 */
		bool TraverseInputNodesToRoot(UEdGraphNode* Node)
		{
			VisitedNodes.Add(Node);

			// Follow every input pin until we cant any more ('root') or we reach a node we have seen (cycle)
			for (int32 PinIndex = 0; PinIndex < Node->Pins.Num(); ++PinIndex)
			{
				UEdGraphPin* MyPin = Node->Pins[PinIndex];

				if (MyPin->Direction == EGPD_Input)
				{
					for (int32 LinkedPinIndex = 0; LinkedPinIndex < MyPin->LinkedTo.Num(); ++LinkedPinIndex)
					{
						UEdGraphPin* OtherPin = MyPin->LinkedTo[LinkedPinIndex];
						if (OtherPin)
						{
							UEdGraphNode* OtherNode = OtherPin->GetOwningNode();
							if (VisitedNodes.Contains(OtherNode))
							{
								return false;
							}
							else
							{
								return TraverseInputNodesToRoot(OtherNode);
							}
						}
					}
				}
			}

			return true;
		}

		TSet<UEdGraphNode*> VisitedNodes;
	};

	// check for cycles
	FNodeVisitorCycleChecker CycleChecker;
	if (!CycleChecker.CheckForLoop(PinA->GetOwningNode(), PinB->GetOwningNode()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorcycle", "Can't create a graph cycle"));
	}

	const bool bPinAIsSingleComposite = (PinA->PinType.PinCategory == UHTNGraphEditorTypes::PinCategory_SingleComposite);
	const bool bPinAIsSingleTask = (PinA->PinType.PinCategory == UHTNGraphEditorTypes::PinCategory_SingleTask);
	const bool bPinAIsSingleNode = (PinA->PinType.PinCategory == UHTNGraphEditorTypes::PinCategory_SingleNode);

	const bool bPinBIsSingleComposite = (PinB->PinType.PinCategory == UHTNGraphEditorTypes::PinCategory_SingleComposite);
	const bool bPinBIsSingleTask = (PinB->PinType.PinCategory == UHTNGraphEditorTypes::PinCategory_SingleTask);
	const bool bPinBIsSingleNode = (PinB->PinType.PinCategory == UHTNGraphEditorTypes::PinCategory_SingleNode);

	const bool bPinASingleLink = bPinAIsSingleComposite || bPinAIsSingleTask || bPinAIsSingleNode;
	const bool bPinBSingleLink = bPinBIsSingleComposite || bPinBIsSingleTask || bPinBIsSingleNode;

	if (PinB->Direction == EGPD_Input && PinB->LinkedTo.Num() > 0)
	{
		if (bPinASingleLink)
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_AB, LOCTEXT("PinConnectReplace", "Replace connection"));
		}
		else
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, LOCTEXT("PinConnectReplace", "Replace connection"));
		}
	}
	else if (PinA->Direction == EGPD_Input && PinA->LinkedTo.Num() > 0)
	{
		if (bPinBSingleLink)
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_AB, LOCTEXT("PinConnectReplace", "Replace connection"));
		}
		else
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, LOCTEXT("PinConnectReplace", "Replace connection"));
		}
	}

	if (bPinASingleLink && PinA->LinkedTo.Num() > 0)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, LOCTEXT("PinConnectReplace", "Replace connection"));
	}
	else if (bPinBSingleLink && PinB->LinkedTo.Num() > 0)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, LOCTEXT("PinConnectReplace", "Replace connection"));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("PinConnect", "Connect nodes"));
}


class FConnectionDrawingPolicy* UEdGraphSchema_HTNGraph::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const
{
	return new FHTNGraphConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}

#undef LOCTEXT_NAMESPACE
