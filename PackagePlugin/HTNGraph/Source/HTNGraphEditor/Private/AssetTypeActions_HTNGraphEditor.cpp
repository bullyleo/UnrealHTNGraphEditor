// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "AssetTypeActions_HTNGraphEditor.h"
#include "HTNGraphEditorModule.h"
#include "HTNGraph.h"
#include "HTNGraphEditor.h"

#include "AIModule.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "AssetTools/Public/IAssetTools.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"


uint32 FAssetTypeActions_HTNGraphEditor::GetCategories() 
{ 
	// Add Newで表示されるアセットのカテゴリを指定
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	return AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("HTN Graph")), LOCTEXT("HTNGraphEditorAssetCategory", "HTNGraph"));
}

void FAssetTypeActions_HTNGraphEditor::OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor )
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for(auto Object : InObjects)
	{
		auto HTNGraph = Cast<UHTNGraph>(Object);
		if(HTNGraph != nullptr)
		{
			bool bFoundExisting = false;
			if(!bFoundExisting)
			{
				FHTNGraphEditorModule& HTNGraphEditorModule = FModuleManager::GetModuleChecked<FHTNGraphEditorModule>( "HTNGraphEditor" );
				TSharedRef< FWorkflowCentricApplication > NewEditor = HTNGraphEditorModule.CreateHTNGraphEditor( Mode, EditWithinLevelEditor, HTNGraph);

			}
		}
	}
}

UClass* FAssetTypeActions_HTNGraphEditor::GetSupportedClass() const
{ 
	return UHTNGraph::StaticClass(); 
}

#undef LOCTEXT_NAMESPACE
