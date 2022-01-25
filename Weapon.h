// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "ArrowType.h"
#include "Engine/DataTable.h"
#include "WeaponType.h"
#include "Weapon.generated.h"

// 데이터 테이블을 사용할땐    Engine/DataTable.h 을 인클루드한다
USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EArrowType ArrowType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 WeaponArrow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ArrowCaseCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USoundCue* PickupSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* EquipSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USkeletalMesh* ItemMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* InventoryIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* ArrowIcon;
    
    // 무기 마다의 크로스 헤어
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CrosshairsMiddle;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CrosshairsLeft;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CrosshairsRight;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CrosshairsBottom;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* CrosshairsTop;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AutoFireRate;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UParticleSystem* BowEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* FireSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeadShotDamage;
    
};


/**
 * 
 */
UCLASS()
class ARCHER_API AWeapon : public AItem
{
	GENERATED_BODY()
public:
    AWeapon();
    
    virtual void Tick(float DeltaTime) override;


protected:
    void StopFalling();
    
    virtual void OnConstruction(const FTransform& Transfrom) override;

private:
    FTimerHandle ThrowWeaponTimer;
    float ThrowWeaponTime;
    bool bFalling;
    
    // 활 무기의 있는 화살을 카운트 한다
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "ture"))
    int32 Arrow;
    
    // 최대로 가질 수 있는 화살을 양을 나타내는 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "ture"))
    int32 ArrowCaseCapacity;
    
    // 무기들의 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "ture"))
    EWeaponType WeaponType;
    
    // 화살들의 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "ture"))
    EArrowType ArrowType;
    
    // 재장전몬타지섹션을 위한 FName 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "ture"))
    FName ReloadMontageSection;
    
    // 무기 프로퍼티의 datatable
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
    UDataTable* WeaponDataTable;
    
    // 무기 마다의 크로스 헤어 텍스쳐
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
    UTexture2D* CrosshairsMiddle;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
    UTexture2D* CrosshairsLeft;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
    UTexture2D* CrosshairsRight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
    UTexture2D* CrosshairsBottom;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
    UTexture2D* CrosshairsTop;
    
    // 활 자동 발사 속도
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
    float AutoFireRate;
    
    // 활을 쏠때 이펙트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
    class UParticleSystem* BowEffect;
    
    // 활을 쏠때 사운드
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
    USoundCue* FireSound;
    
    // 253 345초
    // 화살로 인한 피해량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    float Damage;
    // 화살로 인한 헤드샷 피해량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
    float HeadShotDamage;
    

    


public:
    // 던진 무기에 충돌을 더한다
    void ThrowWeapon();
    
    FORCEINLINE int32 GetArrow() const {return Arrow;}
    FORCEINLINE int32 GetArrowCaseCapacity() const { return ArrowCaseCapacity;}
    
    // 활을 쏠때 캐릭터 클래스를 불러온다
    void DecrementArrow();
    
    FORCEINLINE EWeaponType GetWeaponType() const {return WeaponType;}
    FORCEINLINE EArrowType GetArrowType() const {return ArrowType;}
    FORCEINLINE FName GetReloadMontageSection() const {return ReloadMontageSection;}
    FORCEINLINE float GetAutoFireRate() const {return AutoFireRate;}
    FORCEINLINE UParticleSystem* GetBowEffect() const {return BowEffect;}
    FORCEINLINE USoundCue* GetFireSound() const {return FireSound;}
    FORCEINLINE float GetDamage() const {return Damage;}
    FORCEINLINE float GetHeadShotDamage() const {return HeadShotDamage;}
    
    void ReloadArrow(int32 Amount);
    
    bool ArrowCaseIsFull();
};
