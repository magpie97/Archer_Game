// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

/**
 * 
 */
UCLASS()
class ARCHER_API AEnemyController : public AAIController
{
	GENERATED_BODY()
    
public:
    AEnemyController();
    virtual void OnPossess(APawn* InPawn) override;
    
private:
    
    // 적을 위한 블랙보드 컴포넌트
    UPROPERTY(BlueprintReadWrite, Category = "AI Behavior", meta = (AllowPrivateAccess = "ture"))
    class UBlackboardComponent* BlackboardComponent;
    
    // 적을 위한 비헤이비어 트리 컴포넌트
    UPROPERTY(BlueprintReadWrite, Category = "AI Behavior", meta = (AllowPrivateAccess = "ture"))
    class UBehaviorTreeComponent* BehaviorTreeComponent;
	
    
public:
    FORCEINLINE UBlackboardComponent* GetBlackboardComponent() const {return BlackboardComponent;}
};
