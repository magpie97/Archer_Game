// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSProjectile.h"
#include <Engine/Classes/Camera/CameraComponent.h> // 이거 추가함
#include "ArrowType.h"
#include "ArcherCharacter.generated.h"




UENUM(BlueprintType)
enum class ECombatState : uint8
{
    ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
    ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
    ECS_Reloading UMETA(DisplayName = "Reloading"),
    ECS_Equipping UMETA(DisplayName = "Equipping"),
    ECS_Stunned UMETA(DisplayName = "Stunned"),
    
    ECS_MAX UMETA(DisplayName = "DefulatMAX")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, CurrentSlotIndex, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHighlightIconDelegate, int32, SlotIndex, bool, bStartAnimation);

UCLASS()
class ARCHER_API AArcherCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AArcherCharacter();
    
    // Take combat damage
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
 
//    캐릭터 달리기
//    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Movement: Walking")
//    float SprintSpeedMultiplier;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    void MoveForward(float Value);
    void MoveRight(float Value);
    
    void Sprint(float Value);

    // 마우스 입력 받으면 설정된 속도로 호출됨
    void TurnAtRate(float Rate);
    void LookUpAtRate(float Rate);
    
    // 마우스 X의 움직임에 따라 회전
    void Turn(float Value);
    // 마우스 Y의 움직임에 따라 회전
    void LookUp(float Value);
    
    // 좌클릭 눌렀을때 화살과 효과음 파티클
    void FireWeapon();
    
    // 화살 궤적의 끝의 위치를 가져오기
    bool GetArrowBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult);
    
    
    void AimingButtonPressed();
    void AimingButtonReleased();
    
    void CameraInterpZoom(float DeltaTime);
    
    // 조준에 따라 baseturnrate 및 baselookuprate 설정한다
    void SetLookRates();
    
    
    void CalculateCrosshairSpread(float DeltaTime);
    
    void FireButtonPressed();
    void FireButtonReleased();
    
    void StartFireTimer();
    
    UFUNCTION()
    void AutoFireReset();
    
    // 크로스헤어 아래 아이템에 대한 라인트레이스 위젯에 닿았을때 뜨게한다
    bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);
    
    // 만약 overlappeditemcount > 0 일때  아이템을 추적한다
    void TraceForitems();
    
    // 기본 무기를 생성하고 장착한다
    class AWeapon* SpawnDefaultWeapon();
    
    // 무기를 가져와서 매쉬에 부착한다
    void EquipWeapon(AWeapon* WeaponToEquip, bool bSawpping = false);
    
    // 활 무기를 땅에 떨어트린다
    void DropWeapon();
    
    void SelectButtonPressed();
    void SelectButtonReleased();
    
    // 현재 장착된 무기와 장착할 TraceHitItem
    void SwapWeapon(AWeapon* WeaponToSwap);
    
    // 탄약을 화살로 Map 초기화
    void InitializeArrowMap();
    
    // 활 무기에 화살이 있는지 확인하는 함수
    bool WeaponHasArrow();
    
    // 활 무기를 쏠때 필요한 함수들
    void PlayFireSound();
    void SendArrow();
    void PlayBowFireMontage();
    
    // R키를 누르면 재장전
    void ReloadButtonPressed();
    // 활에 화살을 재장전을 처리한다
    void ReloadWeapon();
    
    // 장착된 무기의 화살 유형이 있는지 확인하는 함수
    bool CarryingArrow();
    
//    void Sprint();
//    void StopSprinting();
    
    void CrouchButtonPressed();
    
    virtual void Jump() override;
    
    // 웅크리거나 서 있을때 캡슐의 절반 높이를 interp 한다
    void InterpCapsuleHalfHeight(float DeltaTime);
    
    void Aim();
    void StopAiming();
    
    void PickupArrow(class AArrow* Arrow);
    
    void FKeyPressed();
    void OneKeyPressed();
    void TwoKeyPressed();
    
    void ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex);
    
    int32 GetEmptyInventorySlot();
    
    void HighlightInventorySlot();
    
    UFUNCTION(BlueprintCallable)
    EPhysicalSurface GetSurfaceType();
    
    UFUNCTION(BlueprintCallable)
    void EndStun();
    
    void Die();
    
    UFUNCTION(BlueprintCallable)
    void FinishDeath();
    
    //UPROPERTY(EditDefaultsOnly, Category = Projectile)
    //TSubclassOf<class AFlyArrow> Projectileclass;
    
    
    
    

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    
private:
    // 카메라붐
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowprivateAccess = "true"))
    class USpringArmComponent* CameraBoom;
    
    // 카메라 캐릭터에 따라오게 하기
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowprivateAccess = "true"))
    class UCameraComponent* FollowCamera;
    
    
    // 기본  회전 속도
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowprivateAccess = "true"))
    float BaseTurnRate;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowprivateAccess = "true"))
    float BaseLookUpRate;
    
    // 조준 안했을때의 마우스감도
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowprivateAccess = "true"))
    float HipTurnRate;
    // 조준 안했을때의 마우스 감도
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowprivateAccess = "true"))
    float HipLookUpRate;
    
    // 조준 했을때의 마우스 감도
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowprivateAccess = "true"))
    float AimingTurnRate;
    
    // 조준 했을때의 마우스 감도
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowprivateAccess = "true"))
    float AimingLookUpRate;
    
    // 마우스 감도의 대한 배율 인수 조준하지 않을때의 상하 감도
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowprivateAccess = "true"),
              meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float MouseHipTurnRate;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowprivateAccess = "true"),
              meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float MouseHipLookUpRate;

    // 마우스 감도의 대한 배율 인주 조준하지 않을때의 좌우 감도
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowprivateAccess = "true"),
              meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float MouseAimingTurnRate;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowprivateAccess = "true"),
              meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float MouseAimingLookUpRate;
    
    
    // 활을 쏠때 몬타지 나오게할것임
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowprivateAccess = "true"))
    class UAnimMontage* HipFireMontage;
    
    // 화살이 벽을 맞으면 파티클이 생성
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowprivateAccess = "true"))
    UParticleSystem* ImpactParticles;
    
    // 화살 궤적을 텍스쳐로 보이게 하기
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowprivateAccess = "true"))
    UParticleSystem* ArrowBeamParticles;
    
    // 조준하기
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowprivateAccess = "true"))
    bool bAiming;
    
    // 기본 카메라 시야 값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowprivateAccess = "true"))
    float CameraDefaultFOV;
    
    // 확대 했을때의 시야 값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowprivateAccess = "true"))
    float CameraZoomedFOV;
    
    // 프레임의 현재 시야
    float CameraCurrentFOV;
    
    // 조준 시 확대/축소를 위한 선형보간 속도 (interpolation)
    // 선형보간이란 쉽게 말해  두 점을 연결하는 방법을 말한다
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowprivateAccess = "true"))
    float ZoomInterpSpeed;
    
    
    // 크로스헤어 만들기
    // 십자모양 크로스헤어를 퍼지게 만든다
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowprivateAccess = "true"))
    float CrosshairSpreadMultiplier;
    // 크로스헤어의 퍼짐의 속도 구성 요소
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowprivateAccess = "true"))
    float CrosshairVelocityFactor;
    // 크로스헤어의 공중에 떠있을때의 퍼짐 구성 요소
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowprivateAccess = "true"))
    float CrosshairInAirFactor;
    // 크로스헤어의 조준하고 있을때의 퍼짐 구성 요소
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowprivateAccess = "true"))
    float CrosshairAimFactor;
    // 크로스헤어의 활을 쏘고 있을때의 퍼짐 구성 요소
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowprivateAccess = "true"))
    float CrosshairShootingFactor;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowprivateAccess = "true"))
    class USoundCue* ChargingSound;
    
    
    
    // fire 버튼 눌렀을때
    bool bFireButtonPressed;
    
    // 값이 true 일때 활을 쏘고 아니라면 기다린다
    bool bShouldFire;
    
    // 활 쏘는 사이의 타이머를 설정한다
    FTimerHandle AutoFireTimer;
    
    // 만약 trace가 trace가 매 프레임마다 아이템을 향한다면 ture 값
    bool bShouldTraceForItems;
    
    // AItem의 겹침의 번호
    int8 OverlappedItemCount;
    
    // 마지막 프레임에 부딪힌 AItem
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowprivateAccess = "true"))
    class AItem* TraceHitItemLastFrame;
    
    // C++ 에서만 쓸려면 VisibleAnywhere
    // 현재 활 무기에서 다른 활 무기로 장착하여 바꾼다
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowprivateAccess = "true"))
    AWeapon* EquippedWeapon;
    
    // EditDefaultsOnly는 UE 에디터에서 기본값을 설정할때
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowprivateAccess = "true"))
    TSubclassOf<AWeapon> DefaultWeaponClass;
    
    // The item currently hit by our trace in trace for items
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowprivateAccess = "true"))
    AItem* TraceHitItem;
    
    // 보간 대상의 카메라에서 바깥쪽 거리
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowprivateAccess = "true"))
    float CameraInterpDistance;
    
    // 보간 대상의 카메라에서 위쪽 거리
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowprivateAccess = "true"))
    float CameraInterpElevation;
    
    
    
    
    
    
    // 다양한 화살 유형의 화살을 추적하기 위한 지도
    //TMap, 맵은 크게 키 유형과 값 유형, 두 가지로 정의되며, 맵에 하나의 짝으로 저장된다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowprivateAccess = "true"))
    TMap<EArrowType, int32> ArrowMap;
    
    // HolyArrow로 장착 후 게임을 시작한다
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowprivateAccess = "true"))
    int32 StartingHolyArrow;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowprivateAccess = "true"))
    int32 StartingIceArrow;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowprivateAccess = "true"))
    int32 StartingNatureArrow;
    
    
    // Combat State, 비어있는 경우에만 발사 또는 재장전하기위한 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowprivateAccess = "true"))
    ECombatState CombatState;
    
    // 재장전 애니메이션을 위한 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowprivateAccess = "true"))
    UAnimMontage* ReloadMontage;
    
    // 재장전 애니메이션을 위한 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowprivateAccess = "true"))
    UAnimMontage* EquipMontage;
    
    UFUNCTION(BlueprintCallable)
    void FinishReloading();
    
    UFUNCTION(BlueprintCallable)
    void FinishEquipping();
    
    // 웅크리기
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowprivateAccess = "true"))
    bool bCrouching;
    
    // 기본 움직임 속도
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowprivateAccess = "true"))
    float BaseMovementSpeed;
    
    // 웅크렸을때의 속도
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowprivateAccess = "true"))
    float CrouchMovementSpeed;
    
    // 달리고 있을때의 속도
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowprivateAccess = "true"))
    float SprintMovementSpeed;
    
    
    
    
    // 이 변수는 interp 를 사용하기 위한 변수들임
    // 현재캐릭터 캡슐의 절반 높이
    float CurrentCapsuleHalfHeight;
    
    // 캐릭터 캡슐의 서있을때의 높이
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowprivateAccess = "true"))
    float StandingCapsuleHalfHeight;
    
    // 캐릭터 캡슐이 웅크리고있을때의 높이
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowprivateAccess = "true"))
    float CrouchingCapsuleHalfHeight;
    
    // 캐릭터의 기본 땅의 마찰 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowprivateAccess = "true"))
    float BaseGroundFriction;
    
    // 캐릭터의 웅크렸을때의 땅의 마찰 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowprivateAccess = "true"))
    float CrouchingGroundFriction;
    
    // 조준 버튼을 눌렀을때를 알때 실행됨
    bool bAimingButtonPressed;
    
    FTimerHandle PickupSoundTimer;
    FTimerHandle EquipSoundTimer;
    
    bool bShouldPlayPickupSound;
    bool bShouldPlayEquipSound;
    
    void ResetPickupSoundTimer();
    void ResetEquipSoundTimer();
    
    // 다른 pickup 사운드를 실행하려면 기다려야 할 시간
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowprivateAccess = "true"))
    float PickupSoundResetTime;
    // 다른 equip 사운드를 실행하려면 기다려야 할 시간
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowprivateAccess = "true"))
    float EquipSoundResetTime;
    
    
    // 아이템의 인벤토리
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowprivateAccess = "ture"))
    TArray<AItem*> Inventory;
    
    const int32 INVENTORY_CAPACITY{ 3 };
    
    // 장착할때 슬롯 정보를 InventoryVar로 보낼 수 있도록 뽑는다
    UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowprivateAccess = "ture"))
    FEquipItemDelegate EquipItemDelegate;
    
    // 아이콘 애니메이션 재생을 위한 슬롯 정보 전송을 위한 델리게이트
    UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
    FHighlightIconDelegate HighlightIconDelegate;
    
    // 현재 강조 표시된 슬롯의 인덱스
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowprivateAccess = "ture"))
    int32 HighlightedSlot;
    
    // 캐릭터 영웅의 체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowprivateAccess = "ture"))
    float Health;
    
    // 캐릭터 영웅의 최대 체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowprivateAccess = "ture"))
    float MaxHealth;
    
    // 캐릭터가 적의 근접공격을 받았을때 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowprivateAccess = "ture"))
    class USoundCue* MeleeImpactSound;
    
    // 캐릭터가 적의 공격ㅇㄹ 받았을때의 피 파티클
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowprivateAccess = "ture"))
    UParticleSystem* BloodParticles;
    
    // 캐릭터가 공격을 맞았을때 스턴걸린다
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowprivateAccess = "ture"))
    UAnimMontage* HitReactMontage;
    
    // 공격을 받아 스턴에 걸릴 확률
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowprivateAccess = "ture"))
    float StunChance;
    
    
    // 캐릭터가 공격을 맞고 죽을때
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowprivateAccess = "ture"))
    UAnimMontage* DeathMontage;
    
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowprivateAccess = "ture"))
    bool bDead;
    

    
    

    
    /*
    public: // 여기는 새로만든 퍼블릭이고 화살 날라가는걸 구현할곳임 안되면 지워버려 슈발
    // 이건 화살 스폰할 위치를 구현
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
    FVector MuzzleOffset;
    
    // FireWeapon() 함수가 호출될때 발사체의 클래스를 담고 있을 클래스와 타입을 만든곳
    UPROPERTY(EditDefaultsOnly, Category = Combat)
    TSubclassOf<AFPSProjectile> ProjectileClass;
    
    */
    
    
public:
    // cameraboom 하위 객체 리턴
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    // followcamera  하위 객체 리턴
    FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}
    
    FORCEINLINE bool GetAiming() const { return bAiming; }
    
    UFUNCTION(BlueprintCallable)
    float GetCrosshairSpreadMultiplier() const;
    
    FORCEINLINE int8 GetOverlappedItemCount() const {return OverlappedItemCount;}
    
    // Overlapped ItemCount에 더하고 빼기 및 bShouldTraceForItems 는 업데이트
    void IncrementOverlappedItemCount(int8 Amount);
    
    FORCEINLINE bool ShouldPlayPickupSound() const { return bShouldPlayEquipSound;}
    FORCEINLINE bool ShouldPlayEquipSound() const { return bShouldPlayPickupSound;}
    

    
    FVector GetCameraInterpLocation();
    
    void GetPickupItem(AItem* Item);
    
    FORCEINLINE ECombatState GetCombatState() const {return CombatState;}
    FORCEINLINE bool GetCrouching() const {return bCrouching;}
    
    void StartPickupSoundTimer();
    void StartEquipSoundTimer();
    
    void UnHighlightInventorySlot();
    
    FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon;}
    FORCEINLINE USoundCue* GetMeleeImpactSound() const {return MeleeImpactSound;}
    FORCEINLINE UParticleSystem* GetBloodParticles() const {return BloodParticles;}
    
    void Stun();
    
    FORCEINLINE float GetStunChance() const {return StunChance;}
};
