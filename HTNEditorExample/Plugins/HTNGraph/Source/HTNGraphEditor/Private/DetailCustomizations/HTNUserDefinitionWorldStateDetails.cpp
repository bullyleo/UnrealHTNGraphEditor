// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HTNUserDefinitionWorldStateDetails.h"
#include "HTNGraphNode_PrimitiveTask.h"
#include "HTNComponent.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/SWidget.h"
#include "PropertyEditing.h"

#define LOCTEXT_NAMESPACE "HTNUserDefinitionWorldStateDetails"

TSharedRef<IPropertyTypeCustomization> FHTNUserDefinitionWorldStateDetails::MakeInstance()
{
	return MakeShareable(new FHTNUserDefinitionWorldStateDetails);
}

void FHTNUserDefinitionWorldStateDetails::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	MyWorldStateKeyValueProperty = StructPropertyHandle->GetChildHandle(TEXT("WorldStateKeyValue"));

	WorldStateKeyValues.Reset();

	TArray<UObject*> MyObjects;
	StructPropertyHandle->GetOuterObjects(MyObjects);
	for (int32 i = 0; i < MyObjects.Num(); i++)
	{
		// このクラスで定義される拡張内容はUHTNGraphNodeクラスとUHTNComponentクラスが持つEnumアセットに対して適用される

		// HTNGraphNodeに設定されたEnumAssetを取得
		UHTNGraphNode* NodeOb = Cast<UHTNGraphNode>(MyObjects[i]);
		if (NodeOb)
		{
			CachedWorldStateEnumAsset = NodeOb->GetTreeAsset()->WorldStateEnumAsset;
			break;
		}

		// HTNComponentに設定されたEnumAssetを取得
		UHTNComponent* Component = Cast<UHTNComponent>(MyObjects[i]);
		if (Component)
		{
			CachedWorldStateEnumAsset = Component->GetWorldStateEnum();
			break;
		}
	}

	if (CachedWorldStateEnumAsset)
	{
		// EnumAssetの項目名を取得しComboBoxのメニューエントリに使用する
		for (int32 i = 0; i < CachedWorldStateEnumAsset->NumEnums() - 1; i++)
		{
			FString DisplayedName = CachedWorldStateEnumAsset->GetDisplayNameTextByIndex(i).ToString();
			WorldStateKeyValues.Add(DisplayedName);
		}
	}
}

void FHTNUserDefinitionWorldStateDetails::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	// WorldStateKeyValue についてはSlateを用いてカスタマイズする
	// これは「ユーザー指定のEnumAsset」の内容を反映したComboBoxを作成し、そこから値を選択する方式にしたいため
	StructBuilder.AddCustomRow(LOCTEXT("TypeRow", "TypeRow"))
		.NameContent()
		[
			MyWorldStateKeyValueProperty->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SComboButton)
			.OnGetMenuContent(this, &FHTNUserDefinitionWorldStateDetails::OnGetKeyContent)
			.IsEnabled(this, &FHTNUserDefinitionWorldStateDetails::IsEditingEnabled)
			.ContentPadding(FMargin(2.0f, 2.0f))
			.ButtonContent()
			[
				SNew(STextBlock)
				.Text(this, &FHTNUserDefinitionWorldStateDetails::GetCurrentKeyDesc)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		];
}

TSharedRef<SWidget> FHTNUserDefinitionWorldStateDetails::OnGetKeyContent() const
{
	FMenuBuilder MenuBuilder(true, NULL);

	for (int32 Idx = 0; Idx < WorldStateKeyValues.Num(); Idx++)
	{
		FUIAction ItemAction(FExecuteAction::CreateSP(const_cast<FHTNUserDefinitionWorldStateDetails*>(this), &FHTNUserDefinitionWorldStateDetails::OnKeyComboChange, Idx));
		MenuBuilder.AddMenuEntry(FText::FromString(WorldStateKeyValues[Idx]), TAttribute<FText>(), FSlateIcon(), ItemAction);
	}

	return MenuBuilder.MakeWidget();
}

FText FHTNUserDefinitionWorldStateDetails::GetCurrentKeyDesc() const
{
	FPropertyAccess::Result Result = FPropertyAccess::Fail;
	int32 CurrentIntValue = INDEX_NONE;

	if (CachedWorldStateEnumAsset)
	{
		Result = MyWorldStateKeyValueProperty->GetValue(CurrentIntValue);
	}

	if (Result == FPropertyAccess::Success && WorldStateKeyValues.IsValidIndex(CurrentIntValue))
		return FText::FromString(WorldStateKeyValues[CurrentIntValue]);
	else
		return FText::GetEmpty();
}

void FHTNUserDefinitionWorldStateDetails::OnKeyComboChange(int32 Index)
{
	MyWorldStateKeyValueProperty->SetValue(Index);
}

#undef LOCTEXT_NAMESPACE
