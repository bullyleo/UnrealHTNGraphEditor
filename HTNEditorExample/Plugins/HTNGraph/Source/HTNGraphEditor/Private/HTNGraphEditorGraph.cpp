// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#include "HTNGraphEditorGraph.h"
#include "HTNGraph.h"
#include "HTNGraphEditorTypes.h"
#include "HTNGraphEditorGraphNode.h"
#include "HTNGraphEditorGraphNode_Composite.h"
#include "HTNGraphEditorGraphNode_Root.h"
#include "HTNGraphEditorGraphNode_PrimitiveTask.h"
#include "HTNGraphEditorGraphNode_Method.h"
#include "EdGraphSchema_HTNGraph.h"
#include "SGraphPanel.h"
#include "EdGraph/EdGraphPin.h"
#include "SGraphNode.h"
#include "AIGraphTypes.h"

//////////////////////////////////////////////////////////////////////////
// BehaviorTreeGraph

namespace HTNGraphVersion
{
	const int32 Initial = 0;
	const int32 Latest = 3;
}

UHTNGraphEditorGraph::UHTNGraphEditorGraph(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Schema = UEdGraphSchema_HTNGraph::StaticClass();
}

// TODO: ‚±‚±‚ÅUHTNGraph‚ÉŠeŽíEnumAsset‚ð“n‚·
void UHTNGraphEditorGraph::InitializeGraphNodeInstance(const UHTNGraphEditorGraphNode_Root* RootNode)
{
	UHTNGraph* HTNGraphAsset = Cast<UHTNGraph>(GetOuter());
	if (HTNGraphAsset == nullptr)
	{
		return;
	}

	for (int32 Index = 0; Index < Nodes.Num(); ++Index)
	{
		UHTNGraphEditorGraphNode* MyNode = Cast<UHTNGraphEditorGraphNode>(Nodes[Index]);

		if (MyNode)
		{
			UHTNGraphNode* MyNodeInstance = Cast<UHTNGraphNode>(MyNode->NodeInstance);
			if (MyNodeInstance)
			{
				HTNGraphAsset->WorldStateEnumAsset = RootNode->WorldStateEnumAsset;
				HTNGraphAsset->OperatorEnumAsset = RootNode->OperatorEnumAsset;
				HTNGraphAsset->OperatorParamEnumAsset = RootNode->OperatorParamEnumAsset;
				MyNodeInstance->InitializeFromAsset(*HTNGraphAsset);
			}
		}
	}
}

void UHTNGraphEditorGraph::UpdateAsset(int32 UpdateFlags)
{
	if (bLockUpdates)
	{
		return;
	}

	// initial cleanup & root node search
	UHTNGraphEditorGraphNode_Root* RootNode = NULL;
	for (int32 Index = 0; Index < Nodes.Num(); ++Index)
	{
		UHTNGraphEditorGraphNode* Node = Cast<UHTNGraphEditorGraphNode>(Nodes[Index]);

		if (Node == nullptr)
		{
			// ignore non-BT nodes.
			continue;
		}

		// prepare node instance
		UHTNGraphNode* NodeInstance = Cast<UHTNGraphNode>(Node->NodeInstance);
		if (NodeInstance)
		{
			// mark all nodes as disconnected first, path from root will replace it with valid values later
			NodeInstance->InitializeNode(NULL, MAX_uint16, 0);
		}

		// cache root
		if (RootNode == NULL)
		{
			RootNode = Cast<UHTNGraphEditorGraphNode_Root>(Nodes[Index]);
		}
	}

	// we can't look at pins until pin references have been fixed up post undo:
	UEdGraphPin::ResolveAllPinReferences();
	if (RootNode && RootNode->Pins.Num() > 0 && RootNode->Pins[0]->LinkedTo.Num() > 0)
	{
		UHTNGraphEditorGraphNode* Node = Cast<UHTNGraphEditorGraphNode>(RootNode->Pins[0]->LinkedTo[0]->GetOwningNode());
		if (Node)
		{
			CreateHTNGraphFromGraph(Node);

			if ((UpdateFlags & KeepRebuildCounter) == 0)
			{
				ModCounter++;
			}
		}
	}

	InitializeGraphNodeInstance(RootNode);
}

void UHTNGraphEditorGraph::OnCreated()
{
	Super::OnCreated();
	SpawnMissingNodes();
}

void UHTNGraphEditorGraph::OnLoaded()
{
	Super::OnLoaded();
	UpdatePinConnectionTypes();
}

void UHTNGraphEditorGraph::OnSave()
{
	UpdateAsset();
}

void UHTNGraphEditorGraph::UpdatePinConnectionTypes()
{
	for (int32 Index = 0; Index < Nodes.Num(); ++Index)
	{
		UEdGraphNode* Node = Nodes[Index];
		check(Node);

		const bool bIsCompositeNode = Node->IsA(UHTNGraphEditorGraphNode_Composite::StaticClass());

		for (int32 iPin = 0; iPin < Node->Pins.Num(); iPin++)
		{
			FName& PinCategory = Node->Pins[iPin]->PinType.PinCategory;
			if (PinCategory == TEXT("Transition"))
			{
				PinCategory = bIsCompositeNode ? 
					UHTNGraphEditorTypes::PinCategory_MultipleNodes :
					UHTNGraphEditorTypes::PinCategory_SingleComposite;
			}
		}
	}
}

void UHTNGraphEditorGraph::ReplaceNodeConnections(UEdGraphNode* OldNode, UEdGraphNode* NewNode)
{
	for (int32 Index = 0; Index < Nodes.Num(); ++Index)
	{
		UEdGraphNode* Node = Nodes[Index];
		check(Node);
		for (int32 PinIndex = 0; PinIndex < Node->Pins.Num(); PinIndex++)
		{
			UEdGraphPin* Pin = Node->Pins[PinIndex];
			for (int32 LinkedIndex = 0; LinkedIndex < Pin->LinkedTo.Num(); LinkedIndex++)
			{
				UEdGraphPin* LinkedPin = Pin->LinkedTo[LinkedIndex];
				const UEdGraphNode* LinkedNode = LinkedPin ? LinkedPin->GetOwningNode() : NULL;

				if (LinkedNode == OldNode)
				{
					check(OldNode);
					check(LinkedPin);

					const int32 LinkedPinIndex = OldNode->Pins.IndexOfByKey(LinkedPin);
					Pin->LinkedTo[LinkedIndex] = NewNode->Pins[LinkedPinIndex];
					LinkedPin->LinkedTo.Remove(Pin);
				}
			}
		}
	}
}

namespace HTNGraphHelpers
{
	struct FIntIntPair
	{
		int32 FirstIdx;
		int32 LastIdx;
	};

	void CreateChildren(UHTNGraph* HTNAsset, UHTNGraphNode_Composite* RootNode, const UHTNGraphEditorGraphNode* RootEdNode, uint16* ExecutionIndex, uint8 TreeDepth)
	{
		if (RootEdNode == NULL)
		{
			return;
		}

		RootNode->Children.Reset();

		// gather all nodes
		int32 ChildIdx = 0;
		for (int32 PinIdx = 0; PinIdx < RootEdNode->Pins.Num(); PinIdx++)
		{
			UEdGraphPin* Pin = RootEdNode->Pins[PinIdx];
			if (Pin->Direction != EGPD_Output)
			{
				continue;
			}

			// sort connections so that they're organized the same as user can see in the editor
			Pin->LinkedTo.Sort(FCompareNodeXLocation());

			for (int32 Index = 0; Index < Pin->LinkedTo.Num(); ++Index)
			{
				UHTNGraphEditorGraphNode* GraphNode = Cast<UHTNGraphEditorGraphNode>(Pin->LinkedTo[Index]->GetOwningNode());
				if (GraphNode == NULL)
				{
					continue;
				}
				UHTNGraphNode_PrimitiveTask* TaskInstance = Cast<UHTNGraphNode_PrimitiveTask>(GraphNode->NodeInstance);
				if (TaskInstance && Cast<UHTNGraph>(TaskInstance->GetOuter()) == NULL)
				{
					TaskInstance->Rename(NULL, HTNAsset);
				}

				UHTNGraphNode_Composite* CompositeInstance = Cast<UHTNGraphNode_Composite>(GraphNode->NodeInstance);
				if (CompositeInstance && Cast<UHTNGraph>(CompositeInstance->GetOuter()) == NULL)
				{
					CompositeInstance->Rename(NULL, HTNAsset);
				}

				UHTNGraphNode_Method* MethodInstance = Cast<UHTNGraphNode_Method>(GraphNode->NodeInstance);
				if (MethodInstance && Cast<UHTNGraph>(MethodInstance->GetOuter()) == NULL)
				{
					MethodInstance->Rename(NULL, HTNAsset);
				}

				if (TaskInstance == NULL && CompositeInstance == NULL && MethodInstance == NULL)
				{
					continue;
				}

				// store child data
				ChildIdx = RootNode->Children.AddDefaulted();
				FHTNGraphCompositeChild& ChildInfo = RootNode->Children[ChildIdx];
				ChildInfo.ChildComposite = CompositeInstance;
				ChildInfo.ChildTask = TaskInstance;

				UHTNGraphNode* ChildNode =	MethodInstance ? (UHTNGraphNode*)MethodInstance :
											CompositeInstance ? (UHTNGraphNode*)CompositeInstance :								(UHTNGraphNode*)TaskInstance;


				// assign execution index to child node
				ChildNode->InitializeNode(RootNode, *ExecutionIndex, TreeDepth);

				*ExecutionIndex += 1;

				if (CompositeInstance)
				{
					CreateChildren(HTNAsset, CompositeInstance, GraphNode, ExecutionIndex, TreeDepth + 1);
					CompositeInstance->InitializeComposite((*ExecutionIndex) - 1);
				}
			}
		}
	}

	void ClearRootLevelFlags(UHTNGraphEditorGraph* Graph)
	{
		for (int32 Index = 0; Index < Graph->Nodes.Num(); Index++)
		{
			UHTNGraphEditorGraphNode* HTNGraphEditorGraphNode = Cast<UHTNGraphEditorGraphNode>(Graph->Nodes[Index]);
			if (HTNGraphEditorGraphNode)
			{
				HTNGraphEditorGraphNode->bRootLevel = false;
			}
		}
	}

	void RebuildExecutionOrder(UHTNGraphEditorGraphNode* RootEdNode, UHTNGraphNode_Composite* RootNode, uint16* ExecutionIndex, uint8 TreeDepth)
	{
		if (RootEdNode == NULL)
		{
			return;
		}

		// gather all nodes
		int32 ChildIdx = 0;
		for (int32 PinIdx = 0; PinIdx < RootEdNode->Pins.Num(); PinIdx++)
		{
			UEdGraphPin* Pin = RootEdNode->Pins[PinIdx];
			if (Pin->Direction != EGPD_Output)
			{
				continue;
			}

			// sort connections so that they're organized the same as user can see in the editor
			TArray<UEdGraphPin*> SortedPins = Pin->LinkedTo;
			SortedPins.Sort(FCompareNodeXLocation());

			for (int32 Index = 0; Index < SortedPins.Num(); ++Index)
			{
				UHTNGraphEditorGraphNode* GraphNode = Cast<UHTNGraphEditorGraphNode>(SortedPins[Index]->GetOwningNode());
				if (GraphNode == NULL)
				{
					continue;
				}

				UHTNGraphNode_PrimitiveTask* TaskInstance = Cast<UHTNGraphNode_PrimitiveTask>(GraphNode->NodeInstance);
				UHTNGraphNode_Composite* CompositeInstance = Cast<UHTNGraphNode_Composite>(GraphNode->NodeInstance);
				UHTNGraphNode* ChildNode = CompositeInstance ? (UHTNGraphNode*)CompositeInstance : (UHTNGraphNode*)TaskInstance;
				if (ChildNode == NULL)
				{
					continue;
				}

				ChildNode->InitializeNode(RootNode, *ExecutionIndex, TreeDepth);
				*ExecutionIndex += 1;
				ChildIdx++;

				if (CompositeInstance)
				{
					RebuildExecutionOrder(GraphNode, CompositeInstance, ExecutionIndex, TreeDepth + 1);
					CompositeInstance->InitializeComposite((*ExecutionIndex) - 1);
				}
			}
		}
	}

	UEdGraphPin* FindGraphNodePin(UEdGraphNode* Node, EEdGraphPinDirection Dir)
	{
		UEdGraphPin* Pin = nullptr;
		for (int32 Idx = 0; Idx < Node->Pins.Num(); Idx++)
		{
			if (Node->Pins[Idx]->Direction == Dir)
			{
				Pin = Node->Pins[Idx];
				break;
			}
		}

		return Pin;
	}


	UHTNGraphEditorGraphNode* SpawnMissingGraphNodesWorker(UHTNGraphNode* Node, UHTNGraphEditorGraphNode* ParentGraphNode, int32 ChildIdx, int32 ParentDecoratorCount, UHTNGraphEditorGraph* Graph)
	{
		if (Node == nullptr)
		{
			return nullptr;
		}

		UHTNGraphEditorGraphNode* GraphNode = nullptr;

		UHTNGraphNode_Composite* CompositeNode = Cast<UHTNGraphNode_Composite>(Node);
		if (CompositeNode)
		{
			FGraphNodeCreator<UHTNGraphEditorGraphNode_Composite> NodeBuilder(*Graph);
			GraphNode = NodeBuilder.CreateNode();
			NodeBuilder.Finalize();
		}
		else
		{
			FGraphNodeCreator<UHTNGraphEditorGraphNode_PrimitiveTask> NodeBuilder(*Graph);
			GraphNode = NodeBuilder.CreateNode();
			NodeBuilder.Finalize();
		}

		if (CompositeNode)
		{
			UEdGraphPin* OutputPin = FindGraphNodePin(GraphNode, EGPD_Output);

			for (int32 Idx = 0; Idx < CompositeNode->Children.Num(); Idx++)
			{
				UHTNGraphNode* ChildNode = CompositeNode->GetChildNode(Idx);
				UHTNGraphEditorGraphNode* ChildGraphNode = SpawnMissingGraphNodesWorker(ChildNode, GraphNode, Idx, ParentDecoratorCount + 0 /*CompositeNode->Children[Idx].Decorators.Num()*/, Graph);

				UEdGraphPin* ChildInputPin = FindGraphNodePin(ChildGraphNode, EGPD_Input);

				OutputPin->MakeLinkTo(ChildInputPin);
			}
		}

		return GraphNode;
	}

	UHTNGraphEditorGraphNode* SpawnMissingGraphNodes(UHTNGraph* Asset, UHTNGraphEditorGraphNode* ParentGraphNode, UHTNGraphEditorGraph* Graph)
	{
		if (ParentGraphNode == nullptr || Asset == nullptr)
		{
			return nullptr;
		}

		UHTNGraphEditorGraphNode* GraphNode = SpawnMissingGraphNodesWorker(Asset->RootNode, ParentGraphNode, 0, /*Asset->RootDecorators.Num()*/ 0, Graph);

		return GraphNode;
	}

} // namespace HTNGraphHelpers

void UHTNGraphEditorGraph::CreateHTNGraphFromGraph(UHTNGraphEditorGraphNode* RootEdNode)
{
	UHTNGraph* HTNGraphAsset = Cast<UHTNGraph>(GetOuter());
	HTNGraphAsset->RootNode = NULL; //discard old tree

	// let's create new tree from graph
	uint16 ExecutionIndex = 0;
	uint8 TreeDepth = 0;

	HTNGraphAsset->RootNode = Cast<UHTNGraphNode_Composite>(RootEdNode->NodeInstance);
	if (HTNGraphAsset->RootNode)
	{
		//HTNGraphAsset->RootNode->InitializeNode(ExecutionIndex);
		HTNGraphAsset->RootNode->InitializeNode(NULL, ExecutionIndex, TreeDepth);

		ExecutionIndex++;
	}

	// collect root level decorators
	uint16 DummyIndex = MAX_uint16;

	// connect tree nodes
	HTNGraphHelpers::CreateChildren(HTNGraphAsset, HTNGraphAsset->RootNode, RootEdNode, &ExecutionIndex, TreeDepth + 1); //-V595

	// mark root level nodes
	HTNGraphHelpers::ClearRootLevelFlags(this);

	RootEdNode->bRootLevel = true;

	if (HTNGraphAsset->RootNode)
	{
		HTNGraphAsset->RootNode->InitializeComposite(ExecutionIndex - 1);
	}

	// Now remove any orphaned nodes left behind after regeneration
	RemoveOrphanedNodes();
}

void UHTNGraphEditorGraph::SpawnMissingNodes()
{
	UHTNGraph* HTNGraphAsset = Cast<UHTNGraph>(GetOuter());
	if (HTNGraphAsset)
	{
		UHTNGraphEditorGraphNode* RootNode = nullptr;
		for (int32 Idx = 0; Idx < Nodes.Num(); Idx++)
		{
			RootNode = Cast<UHTNGraphEditorGraphNode_Root>(Nodes[Idx]);
			if (RootNode)
			{
				break;
			}
		}

		UHTNGraphEditorGraphNode* SpawnedRootNode = HTNGraphHelpers::SpawnMissingGraphNodes(HTNGraphAsset, RootNode, this);
		if (RootNode && SpawnedRootNode)
		{
			UEdGraphPin* RootOutPin = FindGraphNodePin(RootNode, EGPD_Output);
			UEdGraphPin* SpawnedInPin = FindGraphNodePin(SpawnedRootNode, EGPD_Input);

			RootOutPin->MakeLinkTo(SpawnedInPin);
		}
	}
}


void UHTNGraphEditorGraph::RebuildExecutionOrder()
{
	// initial cleanup & root node search
	UHTNGraphEditorGraphNode_Root* RootNode = NULL;
	for (int32 Index = 0; Index < Nodes.Num(); ++Index)
	{
		UHTNGraphEditorGraphNode* Node = Cast<UHTNGraphEditorGraphNode>(Nodes[Index]);
		if (Node == nullptr)
		{
			continue;;
		}

		// prepare node instance
		UHTNGraphNode* NodeInstance = Cast<UHTNGraphNode>(Node->NodeInstance);
		if (NodeInstance)
		{
			// mark all nodes as disconnected first, path from root will replace it with valid values later
			NodeInstance->InitializeNode(NULL, MAX_uint16, 0);
		}

		// cache root
		if (RootNode == NULL)
		{
			RootNode = Cast<UHTNGraphEditorGraphNode_Root>(Nodes[Index]);
		}
	}

	if (RootNode && RootNode->Pins.Num() > 0 && RootNode->Pins[0]->LinkedTo.Num() > 0)
	{
		UHTNGraphEditorGraphNode* Node = Cast<UHTNGraphEditorGraphNode>(RootNode->Pins[0]->LinkedTo[0]->GetOwningNode());
		if (Node)
		{
			UHTNGraphNode_Composite* HTNGraphNode = Cast<UHTNGraphNode_Composite>(Node->NodeInstance);
			if (HTNGraphNode)
			{
				uint16 ExecutionIndex = 0;
				uint8 TreeDepth = 0;

				HTNGraphNode->InitializeNode(NULL, ExecutionIndex, TreeDepth);
				ExecutionIndex++;

				HTNGraphHelpers::RebuildExecutionOrder(Node, HTNGraphNode, &ExecutionIndex, TreeDepth);
			}
		}
	}
}

void UHTNGraphEditorGraph::RebuildChildOrder(UEdGraphNode* ParentNode)
{
	bool bUpdateExecutionOrder = false;
	if (ParentNode)
	{
		for (int32 PinIdx = 0; PinIdx < ParentNode->Pins.Num(); PinIdx++)
		{
			UEdGraphPin* Pin = ParentNode->Pins[PinIdx];
			if (Pin->Direction == EGPD_Output)
			{
				TArray<UEdGraphPin*> PrevOrder(Pin->LinkedTo);
				Pin->LinkedTo.Sort(FCompareNodeXLocation());

				bUpdateExecutionOrder = bUpdateExecutionOrder || (PrevOrder != Pin->LinkedTo);
			}
		}
	}

	if (bUpdateExecutionOrder)
	{
		UpdateAsset(KeepRebuildCounter);
		Modify();
	}
}

#if WITH_EDITOR
void UHTNGraphEditorGraph::PostEditUndo()
{
	Super::PostEditUndo();

	// make sure that all execution indices are up to date
	UpdateAsset(KeepRebuildCounter);
	Modify();
}
#endif // WITH_EDITOR

namespace HTNAutoArrangeHelpers
{
	struct FNodeBoundsInfo
	{
		FVector2D SubGraphBBox;
		TArray<FNodeBoundsInfo> Children;
	};

	void AutoArrangeNodes(UHTNGraphEditorGraphNode* ParentNode, FNodeBoundsInfo& BBoxTree, float PosX, float PosY)
	{
		int32 BBoxIndex = 0;

		UEdGraphPin* Pin = HTNGraphHelpers::FindGraphNodePin(ParentNode, EGPD_Output);
		if (Pin)
		{
			SGraphNode::FNodeSet NodeFilter;
			TArray<UEdGraphPin*> TempLinkedTo = Pin->LinkedTo;
			for (int32 Idx = 0; Idx < TempLinkedTo.Num(); Idx++)
			{
				UHTNGraphEditorGraphNode* GraphNode = Cast<UHTNGraphEditorGraphNode>(TempLinkedTo[Idx]->GetOwningNode());
				if (GraphNode && BBoxTree.Children.Num() > 0)
				{
					AutoArrangeNodes(GraphNode, BBoxTree.Children[BBoxIndex], PosX, PosY + GraphNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().Y * 2.5f);
					GraphNode->DEPRECATED_NodeWidget.Pin()->MoveTo(FVector2D(BBoxTree.Children[BBoxIndex].SubGraphBBox.X / 2 - GraphNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().X / 2 + PosX, PosY), NodeFilter);
					PosX += BBoxTree.Children[BBoxIndex].SubGraphBBox.X + 20;
					BBoxIndex++;
				}

			}
		}
	}

	void GetNodeSizeInfo(UHTNGraphEditorGraphNode* ParentNode, FNodeBoundsInfo& BBoxTree)
	{
		BBoxTree.SubGraphBBox = ParentNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize();
		float LevelWidth = 0;
		float LevelHeight = 0;

		UEdGraphPin* Pin = HTNGraphHelpers::FindGraphNodePin(ParentNode, EGPD_Output);
		if (Pin)
		{
			Pin->LinkedTo.Sort(FCompareNodeXLocation());
			for (int32 Idx = 0; Idx < Pin->LinkedTo.Num(); Idx++)
			{
				UHTNGraphEditorGraphNode* GraphNode = Cast<UHTNGraphEditorGraphNode>(Pin->LinkedTo[Idx]->GetOwningNode());
				if (GraphNode)
				{
					const int32 ChildIdx = BBoxTree.Children.Add(FNodeBoundsInfo());
					FNodeBoundsInfo& ChildBounds = BBoxTree.Children[ChildIdx];

					GetNodeSizeInfo(GraphNode, ChildBounds);

					LevelWidth += ChildBounds.SubGraphBBox.X + 20;
					if (ChildBounds.SubGraphBBox.Y > LevelHeight)
					{
						LevelHeight = ChildBounds.SubGraphBBox.Y;
					}
				}
			}

			if (LevelWidth > BBoxTree.SubGraphBBox.X)
			{
				BBoxTree.SubGraphBBox.X = LevelWidth;
			}

			BBoxTree.SubGraphBBox.Y += LevelHeight;
		}
	}
}

void UHTNGraphEditorGraph::AutoArrange()
{
	UHTNGraphEditorGraphNode* RootNode = nullptr;
	for (int32 Idx = 0; Idx < Nodes.Num(); Idx++)
	{
		RootNode = Cast<UHTNGraphEditorGraphNode_Root>(Nodes[Idx]);
		if (RootNode)
		{
			break;
		}
	}

	if (!RootNode)
	{
		return;
	}

	HTNAutoArrangeHelpers::FNodeBoundsInfo BBoxTree;
	HTNAutoArrangeHelpers::GetNodeSizeInfo(RootNode, BBoxTree);
	HTNAutoArrangeHelpers::AutoArrangeNodes(RootNode, BBoxTree, 0, RootNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().Y * 2.5f);

	RootNode->NodePosX = BBoxTree.SubGraphBBox.X / 2 - RootNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().X / 2;
	RootNode->NodePosY = 0;

	RootNode->DEPRECATED_NodeWidget.Pin()->GetOwnerPanel()->ZoomToFit(/*bOnlySelection=*/ false);
}

void UHTNGraphEditorGraph::UpdateVersion()
{
	if (!bIsUsingModCounter)
	{
		bIsUsingModCounter = true;
		GraphVersion = HTNGraphVersion::Initial;
	}

	if (GraphVersion == HTNGraphVersion::Latest)
	{
		return;
	}

	GraphVersion = HTNGraphVersion::Latest;
	Modify();
}

void UHTNGraphEditorGraph::MarkVersion()
{
	GraphVersion = HTNGraphVersion::Latest;
	bIsUsingModCounter = true;
}
