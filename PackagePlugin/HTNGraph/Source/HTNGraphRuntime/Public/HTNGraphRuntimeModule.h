// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IHTNGraphRuntimeModule.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHTNGraphRuntime, Log, All);

class FHTNGraphRuntimeModule : public IHTNGraphRuntimeModule
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
