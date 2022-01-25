// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GruxAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ARCHER_API UGruxAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
    
    //애니메이션 속성 업데이트
    UFUNCTION(BlueprintCallable)
    void UpdateAnimationProperties(float DeltaTime);
    
private:
    
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "ture"))
    float Speed;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "ture"))
    class AEnemy* Enemy;
	
};
