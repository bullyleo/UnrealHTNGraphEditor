// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Widgets/Input/STextComboBox.h"
#include "UnrealClient.h"
#include "PropertyEditor/Public/IDetailCustomization.h"

class IPropertyHandle;

//--------------------------------------------------------------
// World StateやOperationに用いるEnumアセットの詳細パネルのレイアウトを定義する
//--------------------------------------------------------------
class FHTNPrimitiveTaskCustomDetails : public IDetailCustomization
{
public:
	// 特定の詳細パネルのレイアウトクラスインスタンスを作成する
	static TSharedRef<IDetailCustomization> MakeInstance();

	// 詳細パネルのレイアウトを定義する
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

private:
	// レイアウトパーツの可視性を取得する
	EVisibility GetEnumValueVisibility() const;

	// デリゲート
	void OnTaskOperatorKeyIDChanged();
	void OnOperationParamKeyIDChanged();
	void OnTaskOperatorValueComboChange(int32 Index);
	void OnOperationParamValueComboChange(int32 Index);
	TSharedRef<SWidget> OnGetTaskOperatorEnumValueContent() const;
	TSharedRef<SWidget> OnGetOperationParamEnumValueContent() const;
	FText GetTaskOperatorCurrentEnumValueDesc() const;
	FText GetOperationParamCurrentEnumValueDesc() const;

	TSharedPtr<IPropertyHandle> TaskOperatorValueValueProperty;
	TSharedPtr<IPropertyHandle> TaskOperatorValueIDProperty;
	TSharedPtr<IPropertyHandle> OperationParamValueProperty;
	TSharedPtr<IPropertyHandle> OperationParamKeyIDProperty;

	// レイアウトの構築に必要なアセットのキャッシュ
	UEnum* CachedTaskOperatorEnumAsset;
	UEnum* CachedOperationParamEnumAsset;

	// KeyName によって選択された列挙値
	TArray<FString> TaskOperatorPropValues;
	TArray<FString> OperationParamPropValues;
};
