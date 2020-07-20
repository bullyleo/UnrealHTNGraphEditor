// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HTNComponent.h"
#include "HTNOperator_BlueprintBase.generated.h"


UENUM()
enum class EHTNOperatorResult : uint8
{
	InProgress,
	Success,
	Failed,
	Aborted
};

/**
 * 
 */
UCLASS(Blueprintable)
class HTNGRAPHRUNTIME_API UHTNOperator_BlueprintBase : public UObject
{
	GENERATED_BODY()
	
protected:
	bool bActivate;
	EHTNOperatorResult Result;

	// アクションを初期化

	UFUNCTION(BlueprintImplementableEvent, Category = "HTN")
		void ReceiveInitializeAction(class AAIController* OwnerController, class APawn* ControlledPawn, const uint8 OperatorParam);

	// アクションを実行（ブループリント側で実装）

	UFUNCTION(BlueprintImplementableEvent, Category = "HTN")
		void ReceiveExecuteAction(class AAIController* OwnerController, class APawn* ControlledPawn, const uint8 OperatorParam);

	// アクションの終了（アクションの成功や失敗を知らせる）

	UFUNCTION(BlueprintCallable, Category = "HTN")
		void FinishExecute(bool bSuccess) { Result = bSuccess ? EHTNOperatorResult::Success : EHTNOperatorResult::Failed; }

	// アクションの中断

	UFUNCTION(BlueprintCallable, Category = "HTN")
		void FinishAbort() { Result = EHTNOperatorResult::Aborted; }


	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HTN")
		void GetActualEffects_Internal(TMap<uint8, int32>& WS, class AAIController* OwnerController, APawn* ControlledPawn);


public:
	UHTNOperator_BlueprintBase();

	void Initialize(class AAIController* OwnerController, class APawn* ControlledPawn, const uint8 OperatorParam)
	{
		bActivate = true;
		Result = EHTNOperatorResult::InProgress;
		ReceiveInitializeAction(OwnerController, ControlledPawn, OperatorParam);
	}

	void Execute(class AAIController* OwnerController, class APawn* ControlledPawn, const uint8 OperatorParam)
	{
		Result = EHTNOperatorResult::InProgress;
		ReceiveExecuteAction(OwnerController, ControlledPawn, OperatorParam);
	}


	void GetActualEffects(TMap<uint8, int32>& WS, class AAIController* OwnerController, APawn* ControlledPawn)
	{
		GetActualEffects_Internal(WS, OwnerController, ControlledPawn);
	}

	void Terminate() { bActivate = false; }
	EHTNOperatorResult GetResult() const { return Result; }
	bool IsActivated()const { return bActivate; }

};
