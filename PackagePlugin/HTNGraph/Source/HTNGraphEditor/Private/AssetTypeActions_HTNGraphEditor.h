// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/IToolkitHost.h"
#include "AssetTypeActions_Base.h"


//--------------------------------------------------------------
// HTNGraphEditorアセットについてのアクションを定義する
//--------------------------------------------------------------
class FAssetTypeActions_HTNGraphEditor : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_HTNGraphEditor", "HTN Graph"); }
	// アイコンの背景色
	virtual FColor GetTypeColor() const override { return FColor(0,255,255); }
	// このAssetTypeActionがサポートするアセットクラスを指定する
	virtual UClass* GetSupportedClass() const override;
	// アセットを開いた時に起動するエディタを作成する
	virtual void OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>() ) override;
	// AddNewで表示されるアセットのカテゴリを指定
	virtual uint32 GetCategories() override;
};
