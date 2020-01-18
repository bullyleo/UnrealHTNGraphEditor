// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HTNGraph.h"
#include "HTNGraphNode.h"

//~ HTN Planner Plugin
#include "HTNPlanner.h"
#include "HTNBuilder.h"
//~ HTN Planner Plugin

#include "HTNComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHTNOnAllTaskFinishedSignature);

USTRUCT(BlueprintType)
struct FWorldStateElement
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, Category =" HTN")
		FUserDefinitionWorldState Key;

	// WorldStateÇÃValueÇÕÇ∑Ç◊Çƒint32å^Ç≈àµÇ§ÅBÇÊÇ¡ÇƒTRUEÇÃèÍçáÇÕÅuÇPÅvÇFALSEÇÃèÍçáÇÕÅuÇOÅvÇì¸óÕÇ∑ÇÈÇ±Ç∆Ç…Ç»ÇÈ.
	UPROPERTY(EditAnywhere, Category =" HTN")
		int32 Value;
};


USTRUCT(BlueprintType)
struct FHTNOperatorClassData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category =" HTN")
		int32 OperatorKey;
	UPROPERTY(EditAnywhere, Category =" HTN")
		TSubclassOf<class UHTNOperator_BlueprintBase> Class;
};



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HTNGRAPHRUNTIME_API UHTNComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "HTN")
		UEnum* WorldStateEnumAsset;
	UPROPERTY(EditAnywhere, Category = "HTN")
		TArray<FWorldStateElement> DefaultWorldState;


	UPROPERTY(EditAnywhere, Category = "HTN")
		UEnum* OperatorEnumAsset;
	UPROPERTY(EditDefaultsOnly, Category = "HTN")
		TArray<FHTNOperatorClassData> HTNOperatorClasses;

	UPROPERTY(Transient)
		class AAIController* AIOwnerController;
	UPROPERTY(Transient)
		class APawn* AIOwnerPawn;

	UPROPERTY(BlueprintAssignable, Category = "HTN")
		FHTNOnAllTaskFinishedSignature OnAllTaskFinished;

	TMap<uint8, UHTNOperator_BlueprintBase*> HTNOperators;
	UHTNOperator_BlueprintBase* CurrentOperator;
	FHTNExecutableAction CurrentActionData;
	bool bReplanning;
	bool bInitializedWorldState;

	FHTNWorldState WorldState;
	FHTNResult Result;
	FHTNBuilder_Domain DomainBuilder;
	FHTNPlanner Planner;


public:	
	// Sets default values for this component's properties
	UHTNComponent();

	UFUNCTION(BlueprintCallable, Category = "AI|HTN")
		void BuildHTNDomain(UHTNGraph* HTNAsset);

	UFUNCTION(BlueprintCallable, Category = "AI|HTN")
		void GeneratePlan();

	UFUNCTION(BlueprintCallable, Category = "AI|HTN")
		void ExecuteOperators();

	UEnum* GetWorldStateEnum() const { return WorldStateEnumAsset; }
	UEnum* GetOperatorEnum() const { return OperatorEnumAsset; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
