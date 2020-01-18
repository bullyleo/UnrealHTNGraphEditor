// Fill out your copyright notice in the Description page of Project Settings.


#include "HTNGraphFactory.h"
#include "HTNGraphRuntime/Public/HTNGraph.h"

#define LOCTEXT_NAMESPACE "HTNGraphFactory"

UHTNGraphFactory::UHTNGraphFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UHTNGraph::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UHTNGraphFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UHTNGraph::StaticClass()));
	return NewObject<UHTNGraph>(InParent, Class, Name, Flags);;
}


bool UHTNGraphFactory::CanCreateNew() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE
