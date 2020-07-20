// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HTNGraphEditorTabFactories.h"
#include "HTNGraphEditorTabs.h"
#include "HTNGraphRuntime/Public/HTNGraph.h"
#include "EditorStyleSet.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "HTNGraphEditorFactories"

FHTNGraphDetailsSummoner::FHTNGraphDetailsSummoner(TSharedPtr<class FHTNGraphEditor> InHTNGraphEditorPtr)
	: FWorkflowTabFactory(FHTNGraphEditorTabs::GraphDetailsID, InHTNGraphEditorPtr)
	, HTNGraphEditorPtr(InHTNGraphEditorPtr)
{
	TabLabel = LOCTEXT("HTNGraphDetailsLabel", "Details");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "Kismet.Tabs.Components");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("HTNGraphDetailsView", "Details");
	ViewMenuTooltip = LOCTEXT("HTNGraphDetailsView_ToolTip", "Show the details view");
}

TSharedRef<SWidget> FHTNGraphDetailsSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	check(HTNGraphEditorPtr.IsValid());
	return HTNGraphEditorPtr.Pin()->SpawnProperties();
}

FText FHTNGraphDetailsSummoner::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("HTNGraphDetailsTabTooltip", "The htn graph details tab allows editing of the properties of htn graph nodes");
}


FHTNGraphEditorSummoner::FHTNGraphEditorSummoner(TSharedPtr<class FHTNGraphEditor> InHTNGraphEditorPtr, FOnCreateGraphEditorWidget CreateGraphEditorWidgetCallback)
	: FDocumentTabFactoryForObjects<UEdGraph>(FHTNGraphEditorTabs::GraphEditorID, InHTNGraphEditorPtr)
	, HTNGraphEditorPtr(InHTNGraphEditorPtr)
	, OnCreateGraphEditorWidget(CreateGraphEditorWidgetCallback)
{
}

void FHTNGraphEditorSummoner::OnTabActivated(TSharedPtr<SDockTab> Tab) const
{
	check(HTNGraphEditorPtr.IsValid());
	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());
	HTNGraphEditorPtr.Pin()->OnGraphEditorFocused(GraphEditor);
}

void FHTNGraphEditorSummoner::OnTabRefreshed(TSharedPtr<SDockTab> Tab) const
{
	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());
	GraphEditor->NotifyGraphChanged();
}

TAttribute<FText> FHTNGraphEditorSummoner::ConstructTabNameForObject(UEdGraph* DocumentID) const
{
	return TAttribute<FText>( FText::FromString( DocumentID->GetName() ) );
}

TSharedRef<SWidget> FHTNGraphEditorSummoner::CreateTabBodyForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const
{
	return OnCreateGraphEditorWidget.Execute(DocumentID);
}

const FSlateBrush* FHTNGraphEditorSummoner::GetTabIconForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const
{
	return FEditorStyle::GetBrush("NoBrush");
}

void FHTNGraphEditorSummoner::SaveState(TSharedPtr<SDockTab> Tab, TSharedPtr<FTabPayload> Payload) const
{
	check(HTNGraphEditorPtr.IsValid());
	check(HTNGraphEditorPtr.Pin()->GetHTNGraph());

	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());

	FVector2D ViewLocation;
	float ZoomAmount;
	GraphEditor->GetViewLocation(ViewLocation, ZoomAmount);

	UEdGraph* Graph = FTabPayload_UObject::CastChecked<UEdGraph>(Payload);
	HTNGraphEditorPtr.Pin()->GetHTNGraph()->LastEditedDocuments.Add(FEditedDocumentInfo(Graph, ViewLocation, ZoomAmount));
}

#undef LOCTEXT_NAMESPACE
