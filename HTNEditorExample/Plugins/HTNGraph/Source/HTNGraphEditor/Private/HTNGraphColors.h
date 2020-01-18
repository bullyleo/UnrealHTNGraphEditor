// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class Error;

//--------------------------------------------------------------
// グラフエディタで用いられる各種カラーを定義する
//--------------------------------------------------------------
namespace HTNGraphColors
{
	namespace NodeBody
	{
		const FLinearColor PrimitiveTask(1.0f, 0.05f, 0.05f);
		const FLinearColor Composite(1.0f, 0.5f, 0.0f);
		const FLinearColor Method(0.0f, 1.0f, 0.0f);
		const FLinearColor Default(0.15f, 0.15f, 0.15f);
		const FLinearColor Root(0.5f, 0.5f, 0.5f, 0.1f);
	}

	namespace NodeBorder
	{
		const FLinearColor Inactive(0.08f, 0.08f, 0.08f);
		const FLinearColor Root(0.2f, 0.2f, 0.2f, 0.2f);
		const FLinearColor Selected(1.00f, 0.08f, 0.08f);
		const FLinearColor Disconnected(0.f, 0.f, 0.f);
		const FLinearColor BrokenWithParent(1.f, 0.f, 1.f);

		const FLinearColor PrimitiveTask(1.0f, 0.05f, 0.05f);
		const FLinearColor Composite(1.0f, 0.5f, 0.0f);
		const FLinearColor Method(0.0f, 1.0f, 0.0f);
	}

	namespace Pin
	{
		const FLinearColor Diff(0.9f, 0.2f, 0.15f);
		const FLinearColor Hover(1.0f, 0.7f, 0.0f);
		const FLinearColor Default(0.02f, 0.02f, 0.02f);
		const FLinearColor TaskOnly(0.13f, 0.03f, 0.4f);
		const FLinearColor CompositeOnly(0.02f, 0.02f, 0.02f);
		const FLinearColor SingleNode(0.02f, 0.02f, 0.02f);
	}
	
	namespace Connection
	{
		const FLinearColor Default(1.0f, 1.0f, 1.0f);
	}

	namespace Action
	{
		const FLinearColor DragMarker(1.0f, 1.0f, 0.2f);
	}
}
