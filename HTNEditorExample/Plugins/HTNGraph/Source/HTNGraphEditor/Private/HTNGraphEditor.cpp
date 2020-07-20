// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HTNGraphEditor.h"
#include "HTNGraph.h"
#include "HTNGraphEditorGraph.h"
#include "HTNGraphEditorGraphNode_Root.h"
#include "HTNGraphEditorModule.h"
#include "HTNGraphEditorModes.h"
#include "HTNGraphEditorTabFactories.h"
#include "HTNGraphEditorUtils.h"
#include "EdGraphSchema_HTNGraph.h"

#include "AssetRegistryModule.h"
#include "GraphEditorActions.h"
#include "Kismet2/BlueprintEditorUtils.h"

	
#define LOCTEXT_NAMESPACE "HTNGraphEditor"

const FName FHTNGraphEditor::HTNGraphMode(TEXT("HTNGraph"));

//////////////////////////////////////////////////////////////////////////
FHTNGraphEditor::FHTNGraphEditor()
	: FWorkflowCentricApplication()
{
	OnPackageSavedDelegateHandle = UPackage::PackageSavedEvent.AddRaw(this, &FHTNGraphEditor::OnPackageSaved);
	bCheckDirtyOnAssetSave = true;
	HTNGraph = nullptr;
}


FHTNGraphEditor::~FHTNGraphEditor()
{
	UPackage::PackageSavedEvent.Remove(OnPackageSavedDelegateHandle);
}


void FHTNGraphEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	DocumentManager->SetTabManager(InTabManager);
	FWorkflowCentricApplication::RegisterTabSpawners(InTabManager);
}

void FHTNGraphEditor::RegisterToolbarTab(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
}

void FHTNGraphEditor::InitHTNGraphEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* InObject)
{
	UHTNGraph* HTNGraphToEdit = Cast<UHTNGraph>(InObject);

	if (HTNGraphToEdit != nullptr)
	{
		HTNGraph = HTNGraphToEdit;
	}

	TSharedPtr<FHTNGraphEditor> ThisPtr(SharedThis(this));
	if (!DocumentManager.IsValid())
	{
		DocumentManager = MakeShareable(new FDocumentTracker);
		DocumentManager->Initialize(ThisPtr);

		// Register the document factories
		{
			TSharedRef<FDocumentTabFactory> GraphEditorFactory = MakeShareable(new FHTNGraphEditorSummoner(ThisPtr, FHTNGraphEditorSummoner::FOnCreateGraphEditorWidget::CreateSP(this,&FHTNGraphEditor::CreateGraphEditorWidget)));

			// Also store off a reference to the grapheditor factory so we can find all the tabs spawned by it later.
			GraphEditorTabFactoryPtr = GraphEditorFactory;
			DocumentManager->RegisterDocumentFactory(GraphEditorFactory);
		}
	}

	TArray<UObject*> ObjectsToEdit;
	if (HTNGraph != nullptr)
	{
		ObjectsToEdit.Add(HTNGraph);
	}

	// if we are already editing objects, dont try to recreate the editor from scratch but update the list of objects in edition
	const TArray<UObject*>* EditedObjects = GetObjectsCurrentlyBeingEdited();
	if (EditedObjects == nullptr || EditedObjects->Num() == 0)
	{
		FGraphEditorCommands::Register();

		const TSharedRef<FTabManager::FLayout> DummyLayout = FTabManager::NewLayout("NullLayout")->AddArea(FTabManager::NewPrimaryArea());
		const bool bCreateDefaultStandaloneMenu = true;
		const bool bCreateDefaultToolbar = true;
		InitAssetEditor(Mode, InitToolkitHost, FHTNGraphEditorModule::HTNGraphEditorAppIdentifier, DummyLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, ObjectsToEdit);

		CreateInternalWidgets();

		FHTNGraphEditorModule& HTNGraphEditorModule = FModuleManager::LoadModuleChecked<FHTNGraphEditorModule>("HTNGraphEditor");
		AddMenuExtender(HTNGraphEditorModule.GetMenuExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));

		AddApplicationMode(HTNGraphMode, MakeShareable(new FHTNGraphEditorApplicationMode(SharedThis(this))));
	}
	else
	{
		for (UObject* ObjectToEdit : ObjectsToEdit)
		{
			if (!EditedObjects->Contains(ObjectToEdit))
			{
				AddEditingObject(ObjectToEdit);
			}
		}
	}

	if (HTNGraphToEdit != nullptr)
	{
		SetCurrentMode(HTNGraphMode);
	}

	RegenerateMenusAndToolbars();
}

void FHTNGraphEditor::RestoreHTNGraph()
{
	// Update HTNGraphEditor asset data based on saved graph to have correct data in editor
	UHTNGraphEditorGraph* MyGraph = Cast<UHTNGraphEditorGraph>(HTNGraph->HTNEdGraph);
	const bool bNewGraph = MyGraph == NULL;
	if (MyGraph == NULL)
	{
		HTNGraph->HTNEdGraph = FBlueprintEditorUtils::CreateNewGraph(HTNGraph, TEXT("HTN Graph"), UHTNGraphEditorGraph::StaticClass(), UEdGraphSchema_HTNGraph::StaticClass());
		MyGraph = Cast<UHTNGraphEditorGraph>(HTNGraph->HTNEdGraph);

		// Initialize the HTN editor graph
		const UEdGraphSchema* Schema = MyGraph->GetSchema();
		Schema->CreateDefaultNodesForGraph(*MyGraph);

		MyGraph->OnCreated();
	}
	else
	{
		MyGraph->OnLoaded();
	}

	MyGraph->Initialize();

	TSharedRef<FTabPayload_UObject> Payload = FTabPayload_UObject::Make(MyGraph);
	TSharedPtr<SDockTab> DocumentTab = DocumentManager->OpenDocument(Payload, bNewGraph ? FDocumentTracker::OpenNewDocument : FDocumentTracker::RestorePreviousDocument);

	if (HTNGraph->LastEditedDocuments.Num() > 0)
	{
		TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(DocumentTab->GetContent());
		GraphEditor->SetViewLocation(HTNGraph->LastEditedDocuments[0].SavedViewOffset, HTNGraph->LastEditedDocuments[0].SavedZoomAmount);
	}

	const bool bIncreaseVersionNum = false;
	if (bNewGraph)
	{
		MyGraph->UpdateAsset(UHTNGraphEditorGraph::ClearDebuggerFlags | UHTNGraphEditorGraph::KeepRebuildCounter);
	}
	else
	{
		MyGraph->UpdateAsset(UHTNGraphEditorGraph::KeepRebuildCounter);
	}
}

void FHTNGraphEditor::SaveEditedObjectState()
{
	// Clear currently edited documents
	HTNGraph->LastEditedDocuments.Empty();

	// Ask all open documents to save their state, which will update LastEditedDocuments
	DocumentManager->SaveAllState();
}

FGraphAppearanceInfo FHTNGraphEditor::GetGraphAppearance() const
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText", "HTN Graph");

	return AppearanceInfo;
}

FName FHTNGraphEditor::GetToolkitFName() const
{
	return FName("HTN Graph");
}

FText FHTNGraphEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "HTNGraph");
}

FString FHTNGraphEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "HTNGraph ").ToString();
}


FLinearColor FHTNGraphEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.0f, 0.0f, 0.2f, 0.5f);
}


/** Create new tab for the supplied graph - don't call this directly, call SExplorer->FindTabForGraph.*/
TSharedRef<SGraphEditor> FHTNGraphEditor::CreateGraphEditorWidget(UEdGraph* InGraph)
{
	check(InGraph != NULL);

	if (!GraphEditorCommands.IsValid())
	{
		CreateCommandList();
	}

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FHTNGraphEditor::OnSelectedNodesChanged);
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FHTNGraphEditor::OnNodeTitleCommitted);

	// Make title bar
	TSharedRef<SWidget> TitleBarWidget =
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush(TEXT("Graph.TitleBackground")))
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.FillWidth(1.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("HTNGraphEditorGraphLabel", "HTN Graph"))
				.TextStyle(FEditorStyle::Get(), TEXT("GraphBreadcrumbButtonText"))
			]
		];

	// Make full graph editor
	const bool bGraphIsEditable = InGraph->bEditable;
	return SNew(SGraphEditor)
			.AdditionalCommands(GraphEditorCommands)
			.IsEditable(this, &FHTNGraphEditor::InEditingMode, bGraphIsEditable)
			.Appearance(this, &FHTNGraphEditor::GetGraphAppearance)
			.TitleBar(TitleBarWidget)
			.GraphToEdit(InGraph)
			.GraphEvents(InEvents);
}

bool FHTNGraphEditor::InEditingMode(bool bGraphIsEditable) const
{
	return bGraphIsEditable;
}

TSharedRef<SWidget> FHTNGraphEditor::SpawnProperties()
{
	return
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.HAlign(HAlign_Fill)
		[
			DetailsView.ToSharedRef()
		]
		;
}


void FHTNGraphEditor::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection)
{
	HTNGraphEditorUtils::FPropertySelectionInfo SelectionInfo;
	TArray<UObject*> Selection = HTNGraphEditorUtils::GetSelectionForPropertyEditor(NewSelection, SelectionInfo);

	UHTNGraphEditorGraph* MyGraph = Cast<UHTNGraphEditorGraph>(HTNGraph->HTNEdGraph);

	// グラフ内のノードが１つ選択されていれば
	if (Selection.Num() == 1)
	{
		// 選択中のノードの情報をDetailsパネルに表示
		if (DetailsView.IsValid())
		{
			DetailsView->SetObjects(Selection);
		}
	}
	// 何も選択していなければ
	else if (DetailsView.IsValid())
	{
		// 選択中のノードがなければ
		if (Selection.Num() == 0)
		{
			// ルートノードについての情報をDetailsパネルに表示
			UHTNGraphEditorGraphNode* RootNode = nullptr;
			for (const auto& Node : MyGraph->Nodes)
			{
				RootNode = Cast<UHTNGraphEditorGraphNode_Root>(Node);
				if (RootNode != nullptr)
				{
					break;
				}
			}

			DetailsView->SetObject(RootNode);
		}
		else
		{
			DetailsView->SetObject(nullptr);
		}
	}
}


void FHTNGraphEditor::CreateInternalWidgets()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, false);
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(NULL);
	DetailsView->SetIsPropertyEditingEnabledDelegate(FIsPropertyEditingEnabled::CreateSP(this, &FHTNGraphEditor::IsPropertyEditable));
	DetailsView->OnFinishedChangingProperties().AddSP(this, &FHTNGraphEditor::OnFinishedChangingProperties);
}


bool FHTNGraphEditor::IsPropertyEditable() const
{
	TSharedPtr<SGraphEditor> FocusedGraphEd = UpdateGraphEdPtr.Pin();
	return FocusedGraphEd.IsValid() && FocusedGraphEd->GetCurrentGraph() && FocusedGraphEd->GetCurrentGraph()->bEditable;
}

void FHTNGraphEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == TEXT("BehaviorAsset"))
	{
		UHTNGraphEditorGraph* MyGraph = Cast<UHTNGraphEditorGraph>(HTNGraph->HTNEdGraph);
		MyGraph->UpdateAsset(UHTNGraphEditorGraph::ClearDebuggerFlags);
	}
	HTNGraph->HTNEdGraph->GetSchema()->ForceVisualizationCacheClear();
}

void FHTNGraphEditor::OnPackageSaved(const FString& PackageFileName, UObject* Outer)
{
	UHTNGraphEditorGraph* MyGraph = HTNGraph ? Cast<UHTNGraphEditorGraph>(HTNGraph->HTNEdGraph) : NULL;
	if (MyGraph)
	{
		const bool bUpdated = true;
		if (bUpdated)
		{
			MyGraph->UpdateAsset(UHTNGraphEditorGraph::ClearDebuggerFlags);
		}
	}
}

void FHTNGraphEditor::OnGraphEditorFocused(const TSharedRef<SGraphEditor>& InGraphEditor)
{
	UpdateGraphEdPtr = InGraphEditor;

	FGraphPanelSelectionSet CurrentSelection;
	CurrentSelection = InGraphEditor->GetSelectedNodes();
	OnSelectedNodesChanged(CurrentSelection);
}

void FHTNGraphEditor::SaveAsset_Execute()
{
	if (HTNGraph)
	{
		UHTNGraphEditorGraph* HTNEditorGraph = Cast<UHTNGraphEditorGraph>(HTNGraph->HTNEdGraph);
		if (HTNEditorGraph)
		{
			HTNEditorGraph->OnSave();
		}
	}
	// save it
	FWorkflowCentricApplication::SaveAsset_Execute();
}

FText FHTNGraphEditor::GetLocalizedMode(FName InMode)
{
	static TMap< FName, FText > LocModes;

	if (LocModes.Num() == 0)
	{
		LocModes.Add(HTNGraphMode, LOCTEXT("HTNGraphMode", "HTN Graph"));
	}

	check(InMode != NAME_None);
	const FText* OutDesc = LocModes.Find(InMode);
	check(OutDesc);
	return *OutDesc;
}

void FHTNGraphEditor::FocusWindow(UObject* ObjectToFocusOn)
{
	if (ObjectToFocusOn == HTNGraph)
	{
		SetCurrentMode(HTNGraphMode);
	}

	FWorkflowCentricApplication::FocusWindow(ObjectToFocusOn);
}

void FHTNGraphEditor::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged)
{
	if (NodeBeingChanged)
	{
		static const FText TranscationTitle = FText::FromString(FString(TEXT("Rename Node")));
		const FScopedTransaction Transaction(TranscationTitle);
		NodeBeingChanged->Modify();
		NodeBeingChanged->OnRenameNode(NewText.ToString());
	}
}

FText FHTNGraphEditor::GetToolkitName() const
{
	const UObject* EditingObject = GetCurrentMode() == HTNGraphMode ? (UObject*)HTNGraph : nullptr;
	if (EditingObject != nullptr)
	{
		return FAssetEditorToolkit::GetLabelForObject(EditingObject);
	}

	return FText();
}

FText FHTNGraphEditor::GetToolkitToolTipText() const
{
	const UObject* EditingObject = GetCurrentMode() == HTNGraphMode ? (UObject*)HTNGraph : nullptr;
	if (EditingObject != nullptr)
	{
		return FAssetEditorToolkit::GetToolTipTextForObject(EditingObject);
	}

	return FText();
}

UHTNGraph* FHTNGraphEditor::GetHTNGraph() const
{
	return HTNGraph;
}

#undef LOCTEXT_NAMESPACE
