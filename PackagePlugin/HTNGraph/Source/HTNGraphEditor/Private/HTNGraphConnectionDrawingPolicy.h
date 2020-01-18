// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIGraphConnectionDrawingPolicy.h"

class FSlateWindowElementList;
class UEdGraph;

// グラフエディタの接続についての描画方法を定義する
class HTNGRAPHEDITOR_API FHTNGraphConnectionDrawingPolicy : public FAIGraphConnectionDrawingPolicy
{
public:
	//
	FHTNGraphConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj);

	// FConnectionDrawingPolicy interface 
	virtual void DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, /*inout*/ FConnectionParams& Params) override;
	// End of FConnectionDrawingPolicy interface
};
