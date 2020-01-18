// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SGraphNode_HTNGraph.h"
#include "HTNGraphEditorGraph.h"
#include "HTNGraphEditorGraphNode.h"
#include "HTNGraphEditorGraphNode_Composite.h"
#include "HTNGraphEditorGraphNode_Root.h"
#include "HTNGraphEditorGraphNode_PrimitiveTask.h"
#include "HTNGraphEditorGraphNode_Method.h"
#include "HTNGraphColors.h"
#include "HTNGraphRuntime/Public/HTNGraph.h"
#include "Types/SlateStructs.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "SGraphPanel.h"
#include "SCommentBubble.h"
#include "SGraphPreviewer.h"
#include "SLevelOfDetailBranchNode.h"
#include "IDocumentation.h"
#include "Editor.h"
#include "GraphEditorSettings.h"
#include "NodeFactory.h"

#define LOCTEXT_NAMESPACE "HTNGraphEditor"

namespace
{
	static const bool bShowExecutionIndexInEditorMode = true;
}

//--------------------------------------------------------------
// ノードピンのインスタンス化やカラーを決める
//--------------------------------------------------------------
class SHTNGraphPin : public SGraphPinAI
{
public:
	SLATE_BEGIN_ARGS(SHTNGraphPin){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin);
protected:
	/** @return The color that we should use to draw this pin */
	virtual FSlateColor GetPinColor() const override;
};

void SHTNGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	SGraphPinAI::Construct(SGraphPinAI::FArguments(), InPin);
}

FSlateColor SHTNGraphPin::GetPinColor() const
{
	return 
		GraphPinObj->bIsDiffing ? HTNGraphColors::Pin::Diff :
		IsHovered() ? HTNGraphColors::Pin::Hover :
		(GraphPinObj->PinType.PinCategory == UHTNGraphEditorTypes::PinCategory_SingleComposite) ? HTNGraphColors::Pin::CompositeOnly :
		(GraphPinObj->PinType.PinCategory == UHTNGraphEditorTypes::PinCategory_SingleTask) ? HTNGraphColors::Pin::TaskOnly :
		(GraphPinObj->PinType.PinCategory == UHTNGraphEditorTypes::PinCategory_SingleNode) ? HTNGraphColors::Pin::SingleNode :
		HTNGraphColors::Pin::Default;
}

//--------------------------------------------------------------
// ノード右上に表示されているノード実行順を表すインデックスウィジェット
//--------------------------------------------------------------

class SHTNGraphIndex : public SCompoundWidget
{
public:
	/** Delegate event fired when the hover state of this widget changes */
	DECLARE_DELEGATE_OneParam(FOnHoverStateChanged, bool /* bHovered */);

	/** Delegate used to receive the color of the node, depending on hover state and state of other siblings */
	DECLARE_DELEGATE_RetVal_OneParam(FSlateColor, FOnGetIndexColor, bool /* bHovered */);

	SLATE_BEGIN_ARGS(SHTNGraphIndex){}
		SLATE_ATTRIBUTE(FText, Text)
		SLATE_EVENT(FOnHoverStateChanged, OnHoverStateChanged)
		SLATE_EVENT(FOnGetIndexColor, OnGetIndexColor)
	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs )
	{
		OnHoverStateChangedEvent = InArgs._OnHoverStateChanged;
		OnGetIndexColorEvent = InArgs._OnGetIndexColor;

		const FSlateBrush* IndexBrush = FEditorStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Index"));

		ChildSlot
		[
			SNew(SOverlay)
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				// Add a dummy box here to make sure the widget doesnt get smaller than the brush
				SNew(SBox)
				.WidthOverride(IndexBrush->ImageSize.X)
				.HeightOverride(IndexBrush->ImageSize.Y)
			]
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(IndexBrush)
				.BorderBackgroundColor(this, &SHTNGraphIndex::GetColor)
				.Padding(FMargin(4.0f, 0.0f, 4.0f, 1.0f))
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(InArgs._Text)
					.Font(FEditorStyle::GetFontStyle("BTEditor.Graph.BTNode.IndexText"))
				]
			]
		];
	}

	virtual void OnMouseEnter( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override
	{
		OnHoverStateChangedEvent.ExecuteIfBound(true);
		SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);
	}

	virtual void OnMouseLeave( const FPointerEvent& MouseEvent ) override
	{
		OnHoverStateChangedEvent.ExecuteIfBound(false);
		SCompoundWidget::OnMouseLeave(MouseEvent);
	}

	/** Get the color we use to display the rounded border */
	FSlateColor GetColor() const
	{
		if(OnGetIndexColorEvent.IsBound())
		{
			return OnGetIndexColorEvent.Execute(IsHovered());
		}

		return FSlateColor::UseForeground();
	}

private:
	/** Delegate event fired when the hover state of this widget changes */
	FOnHoverStateChanged OnHoverStateChangedEvent;

	/** Delegate used to receive the color of the node, depending on hover state and state of other siblings */
	FOnGetIndexColor OnGetIndexColorEvent;
};


/////////////////////////////////////////////////////
// SGraphNode_HTNGraph

void SGraphNode_HTNGraph::Construct(const FArguments& InArgs, UHTNGraphEditorGraphNode* InNode)
{
	DebuggerStateDuration = 0.0f;
	DebuggerStateCounter = INDEX_NONE;
	bSuppressDebuggerTriggers = false;

	SGraphNodeAI::Construct(SGraphNodeAI::FArguments(), InNode);
}

FSlateColor SGraphNode_HTNGraph::GetBorderBackgroundColor() const
{
	UHTNGraphEditorGraphNode* HTNGraphNode = Cast<UHTNGraphEditorGraphNode>(GraphNode);
	if (HTNGraphNode->IsA<UHTNGraphEditorGraphNode_Composite>())
		return HTNGraphColors::NodeBorder::Composite;
	if (HTNGraphNode->IsA<UHTNGraphEditorGraphNode_PrimitiveTask>())
		return HTNGraphColors::NodeBorder::PrimitiveTask;
	if (HTNGraphNode->IsA<UHTNGraphEditorGraphNode_Method>())
		return HTNGraphColors::NodeBorder::Method;

	return HTNGraphColors::NodeBorder::Inactive;
}

FSlateColor SGraphNode_HTNGraph::GetBackgroundColor() const
{
	UHTNGraphEditorGraphNode* HTNGraphNode = Cast<UHTNGraphEditorGraphNode>(GraphNode);
	const bool bIsActiveForDebugger = HTNGraphNode ? !bSuppressDebuggerColor : false;

	FLinearColor NodeColor = HTNGraphColors::NodeBody::Default;
	if (Cast<UHTNGraphEditorGraphNode_PrimitiveTask>(GraphNode))
	{
		check(HTNGraphNode);
		NodeColor = HTNGraphColors::NodeBody::PrimitiveTask;
	}
	else if (Cast<UHTNGraphEditorGraphNode_Composite>(GraphNode))
	{
		check(HTNGraphNode);
		NodeColor = HTNGraphColors::NodeBody::Composite;
	}
	else if (Cast<UHTNGraphEditorGraphNode_Method>(GraphNode))
	{
		check(HTNGraphNode);
		NodeColor = HTNGraphColors::NodeBody::Method;
	}

	else if (Cast<UHTNGraphEditorGraphNode_Root>(GraphNode) && GraphNode->Pins.IsValidIndex(0) && GraphNode->Pins[0]->LinkedTo.Num() > 0)
	{
		NodeColor = HTNGraphColors::NodeBody::Root;
	}

	return (FlashAlpha > 0.0f) ? FMath::Lerp(NodeColor, FlashColor, FlashAlpha) : NodeColor;
}

void SGraphNode_HTNGraph::UpdateGraphNode()
{
	bDragMarkerVisible = false;
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();
	SubNodes.Reset();
	OutputPinBox.Reset();

	TSharedPtr<SErrorText> ErrorText;
	TSharedPtr<STextBlock> DescriptionText; 
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	TWeakPtr<SNodeTitle> WeakNodeTitle = NodeTitle;
	auto GetNodeTitlePlaceholderWidth = [WeakNodeTitle]() -> FOptionalSize
	{
		TSharedPtr<SNodeTitle> NodeTitlePin = WeakNodeTitle.Pin();
		const float DesiredWidth = (NodeTitlePin.IsValid()) ? NodeTitlePin->GetTitleSize().X : 0.0f;
		return FMath::Max(75.0f, DesiredWidth);
	};
	auto GetNodeTitlePlaceholderHeight = [WeakNodeTitle]() -> FOptionalSize
	{
		TSharedPtr<SNodeTitle> NodeTitlePin = WeakNodeTitle.Pin();
		const float DesiredHeight = (NodeTitlePin.IsValid()) ? NodeTitlePin->GetTitleSize().Y : 0.0f;
		return FMath::Max(22.0f, DesiredHeight);
	};

	const FMargin NodePadding = FMargin(2.0f);

	IndexOverlay = SNew(SHTNGraphIndex)
		.ToolTipText(this, &SGraphNode_HTNGraph::GetIndexTooltipText)
		.Visibility(this, &SGraphNode_HTNGraph::GetIndexVisibility)
		.Text(this, &SGraphNode_HTNGraph::GetIndexText)
		.OnHoverStateChanged(this, &SGraphNode_HTNGraph::OnIndexHoverStateChanged)
		.OnGetIndexColor(this, &SGraphNode_HTNGraph::GetIndexColor);

	this->ContentScale.Bind( this, &SGraphNode::GetContentScale );
	this->GetOrAddSlot( ENodeZone::Center )
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage( FEditorStyle::GetBrush( "Graph.StateNode.Body" ) )
			.Padding(0.0f)
			.BorderBackgroundColor( this, &SGraphNode_HTNGraph::GetBorderBackgroundColor )
			.OnMouseButtonDown(this, &SGraphNode_HTNGraph::OnMouseDown)
			[
				SNew(SOverlay)

				// Pins and node details
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SVerticalBox)

					// INPUT PIN AREA
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.MinDesiredHeight(NodePadding.Top)
						[
							SAssignNew(LeftNodeBox, SVerticalBox)
						]
					]

					// STATE NAME AREA
					+SVerticalBox::Slot()
					.Padding(FMargin(NodePadding.Left, 0.0f, NodePadding.Right, 0.0f))
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(NodeBody, SBorder)
							.BorderImage( FEditorStyle::GetBrush("BTEditor.Graph.BTNode.Body") )
							.BorderBackgroundColor( this, &SGraphNode_HTNGraph::GetBackgroundColor )
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Center)
							.Visibility(EVisibility::SelfHitTestInvisible)
							[
								SNew(SOverlay)
								+SOverlay::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SNew(SVerticalBox)
									+SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(SHorizontalBox)									
										+SHorizontalBox::Slot()
										.AutoWidth()
										[
											SNew(SLevelOfDetailBranchNode)
											.UseLowDetailSlot(this, &SGraphNode_HTNGraph::UseLowDetailNodeTitles)
											.LowDetail()
											[
												SNew(SBox)
												.WidthOverride_Lambda(GetNodeTitlePlaceholderWidth)
												.HeightOverride_Lambda(GetNodeTitlePlaceholderHeight)
											]
											.HighDetail()
											[
												SNew(SHorizontalBox)
												+SHorizontalBox::Slot()
												.AutoWidth()
												.VAlign(VAlign_Center)
												[
													SNew(SImage)
													.Image(this, &SGraphNode_HTNGraph::GetNameIcon)
												]
												+SHorizontalBox::Slot()
												.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
												[
													SNew(SVerticalBox)
													+SVerticalBox::Slot()
													.AutoHeight()
													[
														SAssignNew(InlineEditableText, SInlineEditableTextBlock)
														.Style( FEditorStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText" )
														.Text( NodeTitle.Get(), &SNodeTitle::GetHeadTitle )
														.OnVerifyTextChanged(this, &SGraphNode_HTNGraph::OnVerifyNameTextChanged)
														.OnTextCommitted(this, &SGraphNode_HTNGraph::OnNameTextCommited)
														.IsReadOnly( this, &SGraphNode_HTNGraph::IsNameReadOnly )
														.IsSelected(this, &SGraphNode_HTNGraph::IsSelectedExclusively)
													]
													+SVerticalBox::Slot()
													.AutoHeight()
													[
														NodeTitle.ToSharedRef()
													]
												]
											]
										]
									]
								]
							]
						]
					]

					// OUTPUT PIN AREA
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.MinDesiredHeight(NodePadding.Bottom)
						[
							SAssignNew(RightNodeBox, SVerticalBox)
							+SVerticalBox::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							.Padding(20.0f,0.0f)
							.FillHeight(1.0f)
							[
								SAssignNew(OutputPinBox, SHorizontalBox)
							]
						]
					]
				]

				// Drag marker overlay
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
				[
					SNew(SBorder)
					.BorderBackgroundColor(HTNGraphColors::Action::DragMarker)
					.ColorAndOpacity(HTNGraphColors::Action::DragMarker)
					.BorderImage(FEditorStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
					.Visibility(this, &SGraphNode_HTNGraph::GetDragOverMarkerVisibility)
					[
						SNew(SBox)
						.HeightOverride(4)
					]
				]
			]
		];
	// Create comment bubble
	TSharedPtr<SCommentBubble> CommentBubble;
	const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

	SAssignNew( CommentBubble, SCommentBubble )
	.GraphNode( GraphNode )
	.Text( this, &SGraphNode::GetNodeComment )
	.OnTextCommitted( this, &SGraphNode::OnCommentTextCommitted )
	.ColorAndOpacity( CommentColor )
	.AllowPinning( true )
	.EnableTitleBarBubble( true )
	.EnableBubbleCtrls( true )
	.GraphLOD( this, &SGraphNode::GetCurrentLOD )
	.IsGraphNodeHovered( this, &SGraphNode::IsHovered );

	GetOrAddSlot( ENodeZone::TopCenter )
	.SlotOffset( TAttribute<FVector2D>( CommentBubble.Get(), &SCommentBubble::GetOffset ))
	.SlotSize( TAttribute<FVector2D>( CommentBubble.Get(), &SCommentBubble::GetSize ))
	.AllowScaling( TAttribute<bool>( CommentBubble.Get(), &SCommentBubble::IsScalingAllowed ))
	.VAlign( VAlign_Top )
	[
		CommentBubble.ToSharedRef()
	];

	CreatePinWidgets();
}

void SGraphNode_HTNGraph::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	SGraphNode::Tick( AllottedGeometry, InCurrentTime, InDeltaTime );
	CachedPosition = AllottedGeometry.AbsolutePosition / AllottedGeometry.Scale;

	DebuggerStateDuration += InDeltaTime;

	float NewFlashAlpha = 0.0f;
	TriggerOffsets.Reset();

	FlashAlpha = NewFlashAlpha;
}

FReply SGraphNode_HTNGraph::OnMouseButtonDoubleClick( const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent )
{
	return SGraphNode::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent );
}

FText SGraphNode_HTNGraph::GetPinTooltip(UEdGraphPin* GraphPinObj) const
{
	FText HoverText = FText::GetEmpty();

	check(GraphPinObj != nullptr);
	UEdGraphNode* OwningGraphNode = GraphPinObj->GetOwningNode();
	if (OwningGraphNode != nullptr)
	{
		FString HoverStr;
		OwningGraphNode->GetPinHoverText(*GraphPinObj, /*out*/HoverStr);
		if (!HoverStr.IsEmpty())
		{
			HoverText = FText::FromString(HoverStr);
		}
	}

	return HoverText;
}

void SGraphNode_HTNGraph::CreatePinWidgets()
{
	UHTNGraphEditorGraphNode* StateNode = CastChecked<UHTNGraphEditorGraphNode>(GraphNode);

	for (int32 PinIdx = 0; PinIdx < StateNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = StateNode->Pins[PinIdx];
		if (!MyPin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SHTNGraphPin, MyPin)
				.ToolTipText( this, &SGraphNode_HTNGraph::GetPinTooltip, MyPin);

			AddPin(NewPin.ToSharedRef());
		}
	}
}

void SGraphNode_HTNGraph::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner( SharedThis(this) );

	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
	const bool bAdvancedParameter = PinObj && PinObj->bAdvancedView;
	if (bAdvancedParameter)
	{
		PinToAdd->SetVisibility( TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced) );
	}

	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		LeftNodeBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(1.0f)
			.Padding(20.0f,0.0f)
			[
				PinToAdd
			];
		InputPins.Add(PinToAdd);
	}
	else // Direction == EEdGraphPinDirection::EGPD_Output
	{
		const bool bIsSingleTaskPin = PinObj && (PinObj->PinType.PinCategory == UHTNGraphEditorTypes::PinCategory_SingleTask);
		if (bIsSingleTaskPin)
		{
			OutputPinBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillWidth(0.4f)
			.Padding(0,0,20.0f,0)
			[
				PinToAdd
			];
		}
		else
		{
			OutputPinBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillWidth(1.0f)
			[
				PinToAdd
			];
		}
		OutputPins.Add(PinToAdd);
	}
}

TSharedPtr<SToolTip> SGraphNode_HTNGraph::GetComplexTooltip()
{
	return IDocumentation::Get()->CreateToolTip(TAttribute<FText>(this, &SGraphNode::GetNodeTooltip), NULL, GraphNode->GetDocumentationLink(), GraphNode->GetDocumentationExcerptName());
}

const FSlateBrush* SGraphNode_HTNGraph::GetNameIcon() const
{	
	UHTNGraphEditorGraphNode* HTNGraphNode = Cast<UHTNGraphEditorGraphNode>(GraphNode);
	return HTNGraphNode != nullptr ? FEditorStyle::GetBrush(HTNGraphNode->GetNameIcon()) : FEditorStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Icon"));
}

static UHTNGraphEditorGraphNode* GetParentNode(UEdGraphNode* GraphNode)
{
	UHTNGraphEditorGraphNode* HTNGraphNode = Cast<UHTNGraphEditorGraphNode>(GraphNode);
	if (HTNGraphNode->ParentNode != nullptr)
	{
		HTNGraphNode = Cast<UHTNGraphEditorGraphNode>(HTNGraphNode->ParentNode);
	}

	UEdGraphPin* MyInputPin = HTNGraphNode->GetInputPin();
	UEdGraphPin* MyParentOutputPin = nullptr;
	if (MyInputPin != nullptr && MyInputPin->LinkedTo.Num() > 0)
	{
		MyParentOutputPin = MyInputPin->LinkedTo[0];
		if(MyParentOutputPin != nullptr)
		{
			if(MyParentOutputPin->GetOwningNode() != nullptr)
			{
				return CastChecked<UHTNGraphEditorGraphNode>(MyParentOutputPin->GetOwningNode());
			}
		}
	}

	return nullptr;
}

void SGraphNode_HTNGraph::OnIndexHoverStateChanged(bool bHovered)
{
	UHTNGraphEditorGraphNode* ParentNode = GetParentNode(GraphNode);
	if(ParentNode != nullptr)
	{
		ParentNode->bHighlightChildNodeIndices = bHovered;
	}
}

FSlateColor SGraphNode_HTNGraph::GetIndexColor(bool bHovered) const
{
	UHTNGraphEditorGraphNode* ParentNode = GetParentNode(GraphNode);
	const bool bHighlightHover = bHovered || (ParentNode && ParentNode->bHighlightChildNodeIndices);

	static const FName HoveredColor("BTEditor.Graph.BTNode.Index.HoveredColor");
	static const FName DefaultColor("BTEditor.Graph.BTNode.Index.Color");

	return bHighlightHover ? FEditorStyle::Get().GetSlateColor(HoveredColor) : FEditorStyle::Get().GetSlateColor(DefaultColor);
}

EVisibility SGraphNode_HTNGraph::GetIndexVisibility() const
{
	// always hide the index on the root node
	if(GraphNode->IsA(UHTNGraphEditorGraphNode_Root::StaticClass()))
	{
		return EVisibility::Collapsed;
	}

	UHTNGraphEditorGraphNode* StateNode = CastChecked<UHTNGraphEditorGraphNode>(GraphNode);
	UEdGraphPin* MyInputPin = StateNode->GetInputPin();
	UEdGraphPin* MyParentOutputPin = NULL;
	if (MyInputPin != NULL && MyInputPin->LinkedTo.Num() > 0)
	{
		MyParentOutputPin = MyInputPin->LinkedTo[0];
	}

	// Visible if we are in PIE or if we have siblings
	CA_SUPPRESS(6235);
	const bool bCanShowIndex = (bShowExecutionIndexInEditorMode || GEditor->bIsSimulatingInEditor || GEditor->PlayWorld != NULL) || (MyParentOutputPin && MyParentOutputPin->LinkedTo.Num() > 1);

	// LOD this out once things get too small
	TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();
	return (bCanShowIndex && (!MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail)) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SGraphNode_HTNGraph::GetIndexText() const
{
	UHTNGraphEditorGraphNode* StateNode = CastChecked<UHTNGraphEditorGraphNode>(GraphNode);
	UEdGraphPin* MyInputPin = StateNode->GetInputPin();
	UEdGraphPin* MyParentOutputPin = NULL;
	if (MyInputPin != NULL && MyInputPin->LinkedTo.Num() > 0)
	{
		MyParentOutputPin = MyInputPin->LinkedTo[0];
	}

	int32 Index = 0;

	CA_SUPPRESS(6235);
	if (bShowExecutionIndexInEditorMode || GEditor->bIsSimulatingInEditor || GEditor->PlayWorld != NULL)
	{
		// show execution index (debugging purposes)
		UHTNGraphNode* HTNNode = Cast<UHTNGraphNode>(StateNode->NodeInstance);
		Index = (HTNNode && HTNNode->GetExecutionIndex() < 0xffff) ? HTNNode->GetExecutionIndex() : -1;
	}
	else
	{
		// show child index
		if (MyParentOutputPin != NULL)
		{
			for (Index = 0; Index < MyParentOutputPin->LinkedTo.Num(); ++Index)
			{
				if (MyParentOutputPin->LinkedTo[Index] == MyInputPin)
				{
					break;
				}
			}
		}
	}

	return FText::AsNumber(Index);
}

FText SGraphNode_HTNGraph::GetIndexTooltipText() const
{
	CA_SUPPRESS(6235);
	if (bShowExecutionIndexInEditorMode || GEditor->bIsSimulatingInEditor || GEditor->PlayWorld != NULL)
	{
		return LOCTEXT("ExecutionIndexTooltip", "Execution index: this shows the order in which nodes are executed.");
	}
	else
	{
		return LOCTEXT("ChildIndexTooltip", "Child index: this shows the order in which child nodes are executed.");
	}
}

TArray<FOverlayWidgetInfo> SGraphNode_HTNGraph::GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const
{
	TArray<FOverlayWidgetInfo> Widgets;

	check(NodeBody.IsValid());
	check(IndexOverlay.IsValid());

	FVector2D Origin(0.0f, 0.0f);

	FOverlayWidgetInfo Overlay(IndexOverlay);
	Overlay.OverlayOffset = FVector2D(WidgetSize.X - (IndexOverlay->GetDesiredSize().X * 0.5f), Origin.Y);
	Widgets.Add(Overlay);

	Origin.Y += NodeBody->GetDesiredSize().Y;

	return Widgets;
}

TSharedRef<SGraphNode> SGraphNode_HTNGraph::GetNodeUnderMouse(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	TSharedPtr<SGraphNode> SubNode = GetSubNodeUnderCursor(MyGeometry, MouseEvent);
	return SubNode.IsValid() ? SubNode.ToSharedRef() : StaticCastSharedRef<SGraphNode>(AsShared());
}

void SGraphNode_HTNGraph::MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter)
{
	SGraphNodeAI::MoveTo(NewPosition, NodeFilter);

	// keep node order (defined by linked pins) up to date with actual positions
	// this function will keep spamming on every mouse move update
	UHTNGraphEditorGraphNode* HTNGraphNode = Cast<UHTNGraphEditorGraphNode>(GraphNode);
	if (HTNGraphNode && !HTNGraphNode->IsSubNode())
	{
		UHTNGraphEditorGraph* HTNGraph = HTNGraphNode->GetHTNGraphEditorGraph();
		if (HTNGraph)
		{
			for (int32 Idx = 0; Idx < HTNGraphNode->Pins.Num(); Idx++)
			{
				UEdGraphPin* Pin = HTNGraphNode->Pins[Idx];
				if (Pin && Pin->Direction == EGPD_Input && Pin->LinkedTo.Num() == 1) 
				{
					UEdGraphPin* ParentPin = Pin->LinkedTo[0];
					if (ParentPin)
					{
						HTNGraph->RebuildChildOrder(ParentPin->GetOwningNode());
					}
				}
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
