// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UnrealClient.h"
#include "IPropertyTypeCustomization.h"

class IPropertyHandle;

// UEnumアセットで定義したWorldStateの内容を詳細パネル上で文字列で表示出来るようにする.

class FHTNUserDefinitionWorldStateDetails : public IPropertyTypeCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	void OnKeyComboChange(int32 Index);
	TSharedRef<SWidget> OnGetKeyContent() const;
	FText GetCurrentKeyDesc() const;
	bool IsEditingEnabled() const { return true; }

	TSharedPtr<IPropertyHandle> MyWorldStateKeyValueProperty;

	UEnum* CachedWorldStateEnumAsset;

	/** cached names of keys */
	TArray<FString> WorldStateKeyValues;
};
