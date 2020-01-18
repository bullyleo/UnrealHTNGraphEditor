// Fill out your copyright notice in the Description page of Project Settings.


#include "HTNComponent.h"
#include "HTNDomain.h"
#include "HTNOperator_BlueprintBase.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"

// Sets default values for this component's properties
UHTNComponent::UHTNComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	bInitializedWorldState = false;
	CurrentOperator = nullptr;
	bReplanning = true;
	// ...
}


// Called when the game starts
void UHTNComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		AIOwnerPawn = Cast<APawn>(OwnerActor);
		if (AIOwnerPawn)
			AIOwnerController = Cast<AAIController>(AIOwnerPawn->GetController());
		else
		{
			AIOwnerController = Cast<AAIController>(OwnerActor);
			AIOwnerPawn = AIOwnerController->GetPawn();
		}
	}

	for (const auto& OperatorClass : HTNOperatorClasses)
	{
		HTNOperators.Add(OperatorClass.OperatorKey, NewObject<UHTNOperator_BlueprintBase>(this, OperatorClass.Class));
	}
}


// Called every frame
void UHTNComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHTNComponent::BuildHTNDomain(UHTNGraph* HTNAsset)
{
	if (bInitializedWorldState == false)
	{
		bInitializedWorldState = true;
		for (const auto& Elem : DefaultWorldState)
		{
			WorldState.SetValueUnsafe((uint16)Elem.Key.WorldStateKeyValue, Elem.Value);
		}
	}

	FName RootName = *HTNAsset->RootNode->NodeName;
	DomainBuilder.SetRootName(RootName);
	HTNAsset->RootNode->ConstructTaskBuilder(DomainBuilder);
	DomainBuilder.Compile();
}


void UHTNComponent::GeneratePlan()
{
	if (DomainBuilder.DomainInstance)
	{
		Planner.GeneratePlan((*DomainBuilder.DomainInstance), WorldState, Result, NAME_None);
		bReplanning = false;

		UE_LOG(LogTemp, Warning, TEXT("---------- Action Plan Result ----------"));
		for (int32 TaskIndex = 0; TaskIndex < Result.TaskIDs.Num(); ++TaskIndex)
		{
			const FHTNPolicy::FTaskID TaskID = Result.TaskIDs[TaskIndex];
			FString NodeName = DomainBuilder.DomainInstance->GetTaskName(TaskID).ToString();
			UE_LOG(LogTemp, Warning, TEXT("%d : %s"), TaskIndex, *NodeName);
		}
		UE_LOG(LogTemp, Warning, TEXT("----------------------------------------"));

		if (Result.TaskIDs.Num() <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Can not found action plan."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Build domain before calling this function."));
	}
}

void UHTNComponent::ExecuteOperators()
{

	if (bReplanning)
	{
		GeneratePlan();
	}

	if (CurrentOperator == nullptr)
	{
		if (Result.ActionsSequence.Num() > 0)
		{
			CurrentActionData = Result.ActionsSequence[0];
			Result.ActionsSequence.RemoveAt(0);
			CurrentOperator = HTNOperators[(uint8)CurrentActionData.ActionID];
		}
		else
		{
			OnAllTaskFinished.Broadcast();
			return;
		}
	}

	if (CurrentOperator)
	{
		if (CurrentOperator->IsActivated() == false)
			CurrentOperator->Initialize(AIOwnerController, AIOwnerPawn, (uint8)CurrentActionData.Parameter);

		if (CurrentOperator->GetResult() == EHTNOperatorResult::InProgress)
		{
			CurrentOperator->Execute(AIOwnerController, AIOwnerPawn, (uint8)CurrentActionData.Parameter);
			return;
		}
		else if (CurrentOperator->GetResult() == EHTNOperatorResult::Failed)
		{
			bReplanning = true;
		}

		CurrentOperator->Terminate();
	}

	TMap<uint8, int32> ActualEffects;
	CurrentOperator->GetActualEffects(ActualEffects, AIOwnerController, AIOwnerPawn);
	if (ActualEffects.Num() > 0)
	{
		for (const auto& State : ActualEffects)
		{
			WorldState.SetValueUnsafe((uint16)State.Key, State.Value);
		}
	}
	CurrentOperator = nullptr;
}

