// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "EdGraph/EdGraph.h"
#include "Widgets/SWidget.h"
#include "WorkflowOrientedApp/WorkflowTabFactory.h"
#include "GraphEditor.h"
#include "HTNGraphEditor.h"
#include "WorkflowOrientedApp/WorkflowUObjectDocuments.h"

//--------------------------------------------------------------
// 詳細パネルタブの生成を行うファクトリ
//--------------------------------------------------------------
struct FHTNGraphDetailsSummoner : public FWorkflowTabFactory
{
public:
	FHTNGraphDetailsSummoner(TSharedPtr<class FHTNGraphEditor> InHTNGraphEditorPtr);

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
	virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const override;

protected:
	TWeakPtr<class FHTNGraphEditor> HTNGraphEditorPtr;
};


//--------------------------------------------------------------
// グラフエディタタブの生成を行うファクトリ
//--------------------------------------------------------------
struct FHTNGraphEditorSummoner : public FDocumentTabFactoryForObjects<UEdGraph>
{
public:
	DECLARE_DELEGATE_RetVal_OneParam(TSharedRef<SGraphEditor>, FOnCreateGraphEditorWidget, UEdGraph*);
public:
	FHTNGraphEditorSummoner(TSharedPtr<class FHTNGraphEditor> InHTNGraphEditorPtr, FOnCreateGraphEditorWidget CreateGraphEditorWidgetCallback);

	virtual void OnTabActivated(TSharedPtr<SDockTab> Tab) const override;
	virtual void OnTabRefreshed(TSharedPtr<SDockTab> Tab) const override;

protected:
	virtual TAttribute<FText> ConstructTabNameForObject(UEdGraph* DocumentID) const override;
	virtual TSharedRef<SWidget> CreateTabBodyForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const override;
	virtual const FSlateBrush* GetTabIconForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const override;
	virtual void SaveState(TSharedPtr<SDockTab> Tab, TSharedPtr<FTabPayload> Payload) const override;

protected:
	TWeakPtr<class FHTNGraphEditor> HTNGraphEditorPtr;
	FOnCreateGraphEditorWidget OnCreateGraphEditorWidget;
};
