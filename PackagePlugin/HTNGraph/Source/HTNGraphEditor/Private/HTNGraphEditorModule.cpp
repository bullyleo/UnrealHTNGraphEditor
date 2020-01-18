// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HTNGraphEditorModule.h"
#include "HTNGraphEditorGraphNode.h"
#include "HTNGraphEditor.h"
#include "AssetTypeActions_HTNGraphEditor.h"
#include "DetailCustomizations/HTNPrimitiveTaskCustomDetails.h"
#include "DetailCustomizations/HTNUserDefinitionWorldStateDetails.h"
#include "DetailCustomizations/HTNOperatorClassDataDetails.h"

#include "SGraphNode_HTNGraph.h"
#include "EdGraphUtilities.h"
#include "SGraphNode.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FHTNGraphModule"

DEFINE_LOG_CATEGORY(LogHTNGraphEditor);

const FName FHTNGraphEditorModule::HTNGraphEditorAppIdentifier(TEXT("HTNGraphEditorApp"));

class FGraphPanelNodeFactory_HTNGraphEditor : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<class SGraphNode> CreateNode(UEdGraphNode* Node) const override
	{
		if (UHTNGraphEditorGraphNode* HTNGraphEditorNode = Cast<UHTNGraphEditorGraphNode>(Node))
		{
			return SNew(SGraphNode_HTNGraph, HTNGraphEditorNode);
		}
		return NULL;
	}
};

TSharedPtr<FGraphPanelNodeFactory> GraphPanelNodeFactory_HTNGraphEditor;

void FHTNGraphEditorModule::StartupModule()
{
	MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
	ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);

	GraphPanelNodeFactory_HTNGraphEditor = MakeShareable(new FGraphPanelNodeFactory_HTNGraphEditor());
	FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory_HTNGraphEditor);

	IAssetTools& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TSharedPtr<FAssetTypeActions_HTNGraphEditor> HTNGraphEditorAssetTypeAction = MakeShareable(new FAssetTypeActions_HTNGraphEditor);
	ItemDataAssetTypeActions.Add(HTNGraphEditorAssetTypeAction);
	AssetToolsModule.RegisterAssetTypeActions(HTNGraphEditorAssetTypeAction.ToSharedRef());

	// HTNGraphNode_Taskをグラフで選択したときに表示される詳細パネルを登録.
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("HTNGraphNode_PrimitiveTask", FOnGetDetailCustomizationInstance::CreateStatic(&FHTNPrimitiveTaskCustomDetails::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout("UserDefinitionWorldState", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FHTNUserDefinitionWorldStateDetails::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout("HTNOperatorClassData", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FHTNOperatorClassDataDetails::MakeInstance));
}


void FHTNGraphEditorModule::ShutdownModule()
{
	if (!UObjectInitialized())
	{
		return;
	}

	MenuExtensibilityManager.Reset();
	ToolBarExtensibilityManager.Reset();
	ClassCache.Reset();

	if (GraphPanelNodeFactory_HTNGraphEditor.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(GraphPanelNodeFactory_HTNGraphEditor);
		GraphPanelNodeFactory_HTNGraphEditor.Reset();
	}

	// Unregister the BehaviorTree item data asset type actions
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (auto& AssetTypeAction : ItemDataAssetTypeActions)
		{
			if (AssetTypeAction.IsValid())
			{
				AssetToolsModule.UnregisterAssetTypeActions(AssetTypeAction.ToSharedRef());
			}
		}
	}
	ItemDataAssetTypeActions.Empty();


	// Unregister the details customization
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout("HTNGraphNode_PrimitiveTask");
		PropertyModule.UnregisterCustomPropertyTypeLayout("UserDefinitionWorldState");
		PropertyModule.UnregisterCustomPropertyTypeLayout("HTNOperatorClassData");
		PropertyModule.NotifyCustomizationModuleChanged();
	}

}

TSharedRef<FWorkflowCentricApplication> FHTNGraphEditorModule::CreateHTNGraphEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UObject* Object)
{
	if (!ClassCache.IsValid())
	{
		ClassCache = MakeShareable(new FGraphNodeClassHelper(UHTNGraphNode::StaticClass()));
		ClassCache->UpdateAvailableBlueprintClasses();
	}

	TSharedRef< FHTNGraphEditor > NewHTNGraphEditor(new FHTNGraphEditor());
	NewHTNGraphEditor->InitHTNGraphEditor(Mode, InitToolkitHost, Object);
	return NewHTNGraphEditor;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FHTNGraphEditorModule, HTNGraphEditor)