// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HTNGraphEditorModes.h"
#include "HTNGraphEditorTabs.h"
#include "HTNGraphEditorTabFactories.h"

#define LOCTEXT_NAMESPACE "HTNGraphApplicationMode"

FHTNGraphEditorApplicationMode::FHTNGraphEditorApplicationMode(TSharedPtr<FHTNGraphEditor> InHTNGraphEditor)
	: FApplicationMode(FHTNGraphEditor::HTNGraphMode, FHTNGraphEditor::GetLocalizedMode)
{
	HTNGraphEditor = InHTNGraphEditor;

	// 「詳細パネル」の作成者クラスを登録
	HTNGraphEditorTabFactories.RegisterFactory(MakeShareable(new FHTNGraphDetailsSummoner(InHTNGraphEditor)));
	
	TabLayout = FTabManager::NewLayout("Standalone_HtnGraphEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->AddTab(InHTNGraphEditor->GetToolbarTabId(), ETabState::OpenedTab)
				->SetHideTabWell(true)
			)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.7f)
					->AddTab(FHTNGraphEditorTabs::GraphEditorID, ETabState::ClosedTab)
				)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.3f)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.6f)
						->AddTab(FHTNGraphEditorTabs::GraphDetailsID, ETabState::OpenedTab)
					)
				)
			)
		);
}

void FHTNGraphEditorApplicationMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	check(HTNGraphEditor.IsValid());
	TSharedPtr<FHTNGraphEditor> HTNGraphEditorPtr = HTNGraphEditor.Pin();
	
	HTNGraphEditorPtr->RegisterToolbarTab(InTabManager.ToSharedRef());

	// Mode-specific setup
	HTNGraphEditorPtr->PushTabFactories(HTNGraphEditorTabFactories);

	FApplicationMode::RegisterTabFactories(InTabManager);
}

void FHTNGraphEditorApplicationMode::PreDeactivateMode()
{
	FApplicationMode::PreDeactivateMode();

	check(HTNGraphEditor.IsValid());
	TSharedPtr<FHTNGraphEditor> HTNGraphEditorPtr = HTNGraphEditor.Pin();
	
	HTNGraphEditorPtr->SaveEditedObjectState();
}

void FHTNGraphEditorApplicationMode::PostActivateMode()
{
	// Reopen any documents that were open when the blueprint was last saved
	check(HTNGraphEditor.IsValid());
	TSharedPtr<FHTNGraphEditor> HTNGraphEditorPtr = HTNGraphEditor.Pin();
	HTNGraphEditorPtr->RestoreHTNGraph();

	FApplicationMode::PostActivateMode();
}

#undef  LOCTEXT_NAMESPACE