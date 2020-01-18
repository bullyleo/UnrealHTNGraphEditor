// Fill out your copyright notice in the Description page of Project Settings.

#include "HTNPrimitiveTaskCustomDetails.h"
#include "HTNGraphNode_PrimitiveTask.h"
#include "HTNGraph.h"
#include "SlateOptMacros.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/STextComboBox.h"


TSharedRef<IDetailCustomization> FHTNPrimitiveTaskCustomDetails::MakeInstance()
{
	return MakeShareable(new FHTNPrimitiveTaskCustomDetails);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void FHTNPrimitiveTaskCustomDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	// 選択したグラフノードを取得
	TArray<TWeakObjectPtr<UObject> > MyOuters;
	DetailLayout.GetObjectsBeingCustomized(MyOuters);
	for (int32 i = 0; i < MyOuters.Num(); i++)
	{
		// 詳細パネルのレイアウトは UHTNGraphNode_PrimitiveTaskクラス が所持するEnumアセットに対してのみ適用される
		UHTNGraphNode_PrimitiveTask* NodeOb = Cast<UHTNGraphNode_PrimitiveTask>(MyOuters[i].Get());
		if (NodeOb)
		{
			CachedTaskOperatorEnumAsset = NodeOb->GetTreeAsset()->OperatorEnumAsset;
			CachedOperationParamEnumAsset = NodeOb->GetTreeAsset()->OperatorParamEnumAsset;

			break;
		}
	}

	if (CachedTaskOperatorEnumAsset == NULL && CachedOperationParamEnumAsset == NULL)
		return;

	// World State Enum Layout
	{
		TaskOperatorValueIDProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UHTNGraph, OperatorEnumAsset), UHTNGraphNode::StaticClass());

		if (TaskOperatorValueIDProperty.IsValid())
		{
			FSimpleDelegate OnKeyChangedDelegate = FSimpleDelegate::CreateSP(this, &FHTNPrimitiveTaskCustomDetails::OnTaskOperatorKeyIDChanged);
			TaskOperatorValueIDProperty->SetOnPropertyValueChanged(OnKeyChangedDelegate);
			OnTaskOperatorKeyIDChanged();
		}

		TaskOperatorValueValueProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UHTNGraphNode_PrimitiveTask, TaskOperatorValue));
		IDetailCategoryBuilder& HTNCategory = DetailLayout.EditCategory("HTN", FText::GetEmpty(), ECategoryPriority::Important);
		IDetailPropertyRow& EnumValueRow = HTNCategory.AddProperty(TaskOperatorValueValueProperty);
		EnumValueRow.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FHTNPrimitiveTaskCustomDetails::GetEnumValueVisibility)));
		EnumValueRow.IsEnabled(true);
		EnumValueRow.CustomWidget()
			.NameContent()
			[
				TaskOperatorValueValueProperty->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				SNew(SComboButton)
				.OnGetMenuContent(this, &FHTNPrimitiveTaskCustomDetails::OnGetTaskOperatorEnumValueContent)
				.ContentPadding(FMargin(2.0f, 2.0f))
				.ButtonContent()
				[
					SNew(STextBlock)
					.Text(this, &FHTNPrimitiveTaskCustomDetails::GetTaskOperatorCurrentEnumValueDesc)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			];
	}

	// Operation Value
	{
		OperationParamKeyIDProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UHTNGraph, OperatorParamEnumAsset), UHTNGraphNode::StaticClass());

		if (OperationParamKeyIDProperty.IsValid())
		{
			FSimpleDelegate OnKeyChangedDelegate = FSimpleDelegate::CreateSP(this, &FHTNPrimitiveTaskCustomDetails::OnOperationParamKeyIDChanged);
			OperationParamKeyIDProperty->SetOnPropertyValueChanged(OnKeyChangedDelegate);
			OnOperationParamKeyIDChanged();
		}

		OperationParamValueProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UHTNGraphNode_PrimitiveTask, OperationParamValue));
		IDetailCategoryBuilder& HTNCategory = DetailLayout.EditCategory("HTN", FText::GetEmpty(), ECategoryPriority::Important);
		IDetailPropertyRow& EnumValueRow = HTNCategory.AddProperty(OperationParamValueProperty);
		EnumValueRow.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FHTNPrimitiveTaskCustomDetails::GetEnumValueVisibility)));
		EnumValueRow.IsEnabled(true);
		EnumValueRow.CustomWidget()
			.NameContent()
			[
				OperationParamValueProperty->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				SNew(SComboButton)
				.OnGetMenuContent(this, &FHTNPrimitiveTaskCustomDetails::OnGetOperationParamEnumValueContent)
				.ContentPadding(FMargin(2.0f, 2.0f))
				.ButtonContent()
				[
					SNew(STextBlock)
					.Text(this, &FHTNPrimitiveTaskCustomDetails::GetOperationParamCurrentEnumValueDesc)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			];
	}

}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION


EVisibility FHTNPrimitiveTaskCustomDetails::GetEnumValueVisibility() const
{
	// Visible
	return EVisibility();
}

void FHTNPrimitiveTaskCustomDetails::OnTaskOperatorKeyIDChanged()
{
	if (CachedTaskOperatorEnumAsset)
	{
		TaskOperatorPropValues.Reset();

		for (int32 i = 0; i < CachedTaskOperatorEnumAsset->NumEnums() - 1; i++)
		{
			FString DisplayedName = CachedTaskOperatorEnumAsset->GetDisplayNameTextByIndex(i).ToString();
			TaskOperatorPropValues.Add(DisplayedName);
		}

		TaskOperatorPropValues.Add(FString("NONE"));
	}
}


void FHTNPrimitiveTaskCustomDetails::OnOperationParamKeyIDChanged()
{
	if (CachedOperationParamEnumAsset)
	{
		OperationParamPropValues.Reset();

		for (int32 i = 0; i < CachedOperationParamEnumAsset->NumEnums() - 1; i++)
		{
			FString DisplayedName = CachedOperationParamEnumAsset->GetDisplayNameTextByIndex(i).ToString();
			OperationParamPropValues.Add(DisplayedName);
		}

		OperationParamPropValues.Add(FString("NONE"));
	}
}


void FHTNPrimitiveTaskCustomDetails::OnTaskOperatorValueComboChange(int32 Index)
{
	TaskOperatorValueValueProperty->SetValue(Index);
}

void FHTNPrimitiveTaskCustomDetails::OnOperationParamValueComboChange(int32 Index)
{
	OperationParamValueProperty->SetValue(Index);
}


TSharedRef<SWidget> FHTNPrimitiveTaskCustomDetails::OnGetTaskOperatorEnumValueContent() const
{
	FMenuBuilder MenuBuilder(true, NULL);

	for (int32 Idx = 0; Idx < TaskOperatorPropValues.Num(); Idx++)
	{
		bool IsValid = TaskOperatorPropValues[Idx] != "NONE";
		FUIAction ItemAction(FExecuteAction::CreateSP(const_cast<FHTNPrimitiveTaskCustomDetails*>(this), &FHTNPrimitiveTaskCustomDetails::OnTaskOperatorValueComboChange, IsValid ? Idx : INDEX_NONE));
		MenuBuilder.AddMenuEntry(FText::FromString(TaskOperatorPropValues[Idx]), TAttribute<FText>(), FSlateIcon(), ItemAction);
	}

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> FHTNPrimitiveTaskCustomDetails::OnGetOperationParamEnumValueContent() const
{
	FMenuBuilder MenuBuilder(true, NULL);

	for (int32 Idx = 0; Idx < OperationParamPropValues.Num(); Idx++)
	{
		bool IsValid = OperationParamPropValues[Idx] != "NONE";
		FUIAction ItemAction(FExecuteAction::CreateSP(const_cast<FHTNPrimitiveTaskCustomDetails*>(this), &FHTNPrimitiveTaskCustomDetails::OnOperationParamValueComboChange, IsValid ? Idx : INDEX_NONE));
		MenuBuilder.AddMenuEntry(FText::FromString(OperationParamPropValues[Idx]), TAttribute<FText>(), FSlateIcon(), ItemAction);
	}

	return MenuBuilder.MakeWidget();
}



FText FHTNPrimitiveTaskCustomDetails::GetTaskOperatorCurrentEnumValueDesc() const
{
	FPropertyAccess::Result Result = FPropertyAccess::Fail;
	int32 CurrentIntValue = INDEX_NONE;

	if (CachedTaskOperatorEnumAsset)
	{
		Result = TaskOperatorValueValueProperty->GetValue(CurrentIntValue);
	}

	return (Result == FPropertyAccess::Success && TaskOperatorPropValues.IsValidIndex(CurrentIntValue))
		? FText::FromString(TaskOperatorPropValues[CurrentIntValue])
		: FText::FromString("NONE");
		//: FText::GetEmpty();
}


FText FHTNPrimitiveTaskCustomDetails::GetOperationParamCurrentEnumValueDesc() const
{
	FPropertyAccess::Result Result = FPropertyAccess::Fail;
	int32 CurrentIntValue = INDEX_NONE;

	if (CachedOperationParamEnumAsset)
	{
		Result = OperationParamValueProperty->GetValue(CurrentIntValue);
	}

	return (Result == FPropertyAccess::Success && OperationParamPropValues.IsValidIndex(CurrentIntValue))
		? FText::FromString(OperationParamPropValues[CurrentIntValue])
		: FText::FromString("NONE");
		//: FText::GetEmpty();
}
