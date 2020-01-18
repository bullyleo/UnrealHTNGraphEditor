// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "AIGraph/Public/AIGraphEditor.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"

//--------------------------------------------------------------
// グラフエディタ（ノードを配置したりするエリア）の動作やレイアウトを定義する
//--------------------------------------------------------------
class FHTNGraphEditor : public FWorkflowCentricApplication, public FAIGraphEditor
{
public:
	FHTNGraphEditor();
	virtual ~FHTNGraphEditor();

	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;

	void InitHTNGraphEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* InObject);

	//~ Begin IToolkit Interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	//~ End IToolkit Interface

	virtual void FocusWindow(UObject* ObjectToFocusOn = NULL) override;

	//~ Begin Delegates
	void OnGraphEditorFocused(const TSharedRef<SGraphEditor>& InGraphEditor);
	void OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged);

	bool IsPropertyEditable() const;
	void OnPackageSaved(const FString& PackageFileName, UObject* Outer);
	void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent);
	//~ End Delegates

	FGraphAppearanceInfo GetGraphAppearance() const;
	bool InEditingMode(bool bGraphIsEditable) const;

	EVisibility GetRootLevelNodeVisibility() const { return EVisibility::Visible; }
	EVisibility GetTextComboBoxVisibility() const { return EVisibility::Visible; }

	TWeakPtr<SGraphEditor> GetFocusedGraphPtr() const { return UpdateGraphEdPtr; }

	/** Check whether the htn graph mode can be accessed */
	bool CanAccessHTNGraphMode() const { return HTNGraph != nullptr; }

	/** Get the localized text to display for the specified mode */
	static FText GetLocalizedMode(FName InMode);

	/** Get the htn graph we are editing (if any) */
	class UHTNGraph* GetHTNGraph() const;

	/** Spawns the tab with the update graph inside */
	TSharedRef<SWidget> SpawnProperties();

	// @todo This is a hack for now until we reconcile the default toolbar with application modes [duplicated from counterpart in Blueprint Editor]
	void RegisterToolbarTab(const TSharedRef<class FTabManager>& TabManager);

	/** Restores the htn graph we were editing or creates a new one if none is available */
	void RestoreHTNGraph();

	/** Save the graph state for later editing */
	void SaveEditedObjectState();

protected:
	/** Called when "Save" is clicked for this asset */
	virtual void SaveAsset_Execute() override;


private:
	/** Create widget for graph editing */
	TSharedRef<class SGraphEditor> CreateGraphEditorWidget(UEdGraph* InGraph);

	/** Creates all internal widgets for the tabs to point at */
	void CreateInternalWidgets();

	/** Called when the selection changes in the GraphEditor */
	virtual void OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection) override;

	TSharedPtr<FDocumentTracker> DocumentManager;
	TWeakPtr<FDocumentTabFactory> GraphEditorTabFactoryPtr;

	/* The HTN graph being edited */
	class UHTNGraph* HTNGraph;

	/** Property View */
	TSharedPtr<class IDetailsView> DetailsView;

	/** Handle to the registered OnPackageSave delegate */
	FDelegateHandle OnPackageSavedDelegateHandle;

public:
	/** Modes in mode switcher */
	static const FName HTNGraphMode;

};
