// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HTNOperatorClassDataDetails.h"
#include "HTNGraphNode_PrimitiveTask.h"
#include "HTNGraph.h"
#include "HTNComponent.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/SWidget.h"
#include "Engine/GameViewportClient.h"
#include "Editor/EditorPerProjectUserSettings.h"
#include "Textures/SlateIcon.h"
#include "Framework/Commands/UIAction.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "IPropertyUtilities.h"
#include "PropertyEditing.h"

#define LOCTEXT_NAMESPACE "HTNUserDefinitionOperatorDetails"

TSharedRef<IPropertyTypeCustomization> FHTNOperatorClassDataDetails::MakeInstance()
{
	return MakeShareable(new FHTNOperatorClassDataDetails);
}

void FHTNOperatorClassDataDetails::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	OperatorKeyPropertyHandle = StructPropertyHandle->GetChildHandle(TEXT("OperatorKey"));
	ClassPropertyHandle = StructPropertyHandle->GetChildHandle(TEXT("Class"));

	OperatorValues.Reset();

	TArray<UObject*> MyObjects;
	StructPropertyHandle->GetOuterObjects(MyObjects);
	for (int32 i = 0; i < MyObjects.Num(); i++)
	{
		// 詳細パネルのレイアウトは UHTNComponentクラス が所持するEnumアセットに対して適用される
		UHTNComponent* Component = Cast<UHTNComponent>(MyObjects[i]);
		if (Component)
		{
			CachedOperatorEnumAsset = Component->GetOperatorEnum();
			break;
		}
	}

	if (CachedOperatorEnumAsset)
	{
		// EnumAssetの項目名を取得しComboBoxのメニューエントリに使用する
		for (int32 i = 0; i < CachedOperatorEnumAsset->NumEnums() - 1; i++)
		{
			FString DisplayedName = CachedOperatorEnumAsset->GetDisplayNameTextByIndex(i).ToString();
			OperatorValues.Add(DisplayedName);
		}
	}
}

void FHTNOperatorClassDataDetails::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	// WorldStateKeyValue についてはSlateを用いてカスタマイズする
	// これは「ユーザー指定のEnumAsset」の内容を反映したComboBoxを作成し、そこから値を選択する方式にしたいため
	StructBuilder.AddCustomRow(LOCTEXT("TypeRow", "TypeRow"))
		.NameContent()
		[
			OperatorKeyPropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SComboButton)
			.OnGetMenuContent(this, &FHTNOperatorClassDataDetails::OnGetKeyContent)
			.IsEnabled(this, &FHTNOperatorClassDataDetails::IsEditingEnabled)
			.ContentPadding(FMargin(2.0f, 2.0f))
			.ButtonContent()
			[
				SNew(STextBlock)
				.Text(this, &FHTNOperatorClassDataDetails::GetCurrentKeyDesc)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		];


	StructBuilder.AddProperty(ClassPropertyHandle.ToSharedRef());
}

TSharedRef<SWidget> FHTNOperatorClassDataDetails::OnGetKeyContent() const
{
	FMenuBuilder MenuBuilder(true, NULL);

	for (int32 Idx = 0; Idx < OperatorValues.Num(); Idx++)
	{
		FUIAction ItemAction(FExecuteAction::CreateSP(const_cast<FHTNOperatorClassDataDetails*>(this), &FHTNOperatorClassDataDetails::OnKeyComboChange, Idx));
		MenuBuilder.AddMenuEntry(FText::FromString(OperatorValues[Idx]), TAttribute<FText>(), FSlateIcon(), ItemAction);
	}

	return MenuBuilder.MakeWidget();
}

FText FHTNOperatorClassDataDetails::GetCurrentKeyDesc() const
{
	FPropertyAccess::Result Result = FPropertyAccess::Fail;
	int32 CurrentIntValue = INDEX_NONE;

	if (CachedOperatorEnumAsset)
	{
		Result = OperatorKeyPropertyHandle->GetValue(CurrentIntValue);
	}

	return (Result == FPropertyAccess::Success && OperatorValues.IsValidIndex(CurrentIntValue))
		? FText::FromString(OperatorValues[CurrentIntValue])
		: FText::GetEmpty();
}

void FHTNOperatorClassDataDetails::OnKeyComboChange(int32 Index)
{
	OperatorKeyPropertyHandle->SetValue(Index);
}

bool FHTNOperatorClassDataDetails::IsEditingEnabled() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE
