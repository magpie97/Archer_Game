// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "ArrowType.h"
#include "Arrow.generated.h"

/**
 * 
 */
UCLASS()
class ARCHER_API AArrow : public AItem
{
	GENERATED_BODY()
    
public:
    AArrow();
    
    virtual void Tick(float DeltaTime) override;
    
protected:
    
    virtual void BeginPlay() override;
    
    // ArrowMesh 속성을 설정할 수 있도록 SetItemProperties를 재정의 한다
    virtual void SetItemProperties(EItemState State) override;
    
    // AreaSphere를 겹칠 때 호출됨
    UFUNCTION()
    void ArrowSphereOverlap(UPrimitiveComponent* OverLappedComponent,
                         AActor* OtherActor,
                         UPrimitiveComponent* OtherComp,
                         int32 OtherBodyIndex,
                         bool bFromSweep,
                         const FHitResult& SweepResult);

    
private:
    // 화살의 매쉬
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Arrow, meta = (AllowprivateAccess = "true"))
    UStaticMeshComponent* ArrowMesh;
    
    // 화살의 종류
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Arrow, meta = (AllowprivateAccess = "true"))
    EArrowType ArrowType;
    
    // 화살 아이콘의 텍스쳐
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Arrow, meta = (AllowprivateAccess = "true"))
    UTexture2D* ArrowIconTexture;
    
    // 화살의 콜리전 구를 만든다
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Arrow, meta = (AllowprivateAccess = "true"))
    class USphereComponent* ArrowCollisionSphere;
    
public:
    FORCEINLINE UStaticMeshComponent* GetArrowMesh() const {return ArrowMesh;}
    FORCEINLINE EArrowType GetArrowType() const {return ArrowType;}
};
