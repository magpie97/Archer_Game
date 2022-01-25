// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ArrowHitInterface.h"
#include "Enemy.generated.h"

UCLASS()                                    // 250 번
class ARCHER_API AEnemy : public ACharacter, public IArrowHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    
    UFUNCTION(BlueprintNativeEvent)
    void ShowHealthBar();
    void ShowHealthBar_Implementation();
    
    UFUNCTION(BlueprintImplementableEvent)
    void HideHealthBar();
    
    void Die();
    
    void PlayHitMontage(FName Section, float PlayRate = 1.0f);
    
    void ResetHitReactTimer();
    
    UFUNCTION(BlueprintCallable)
    void StoreHitNumber(UUserWidget* HitNumber, FVector Location);
    
    UFUNCTION()
    void DestroyHitNumber(UUserWidget* HitNumber);
    
    void UpdateHitNumbers();
    
    // Agrosphere가 무언가에 겹칠때 호출된다
    UFUNCTION()
    void AgroSphereOverlap(UPrimitiveComponent* OverLappedComponent,
                           AActor* OtherActor,
                           UPrimitiveComponent* OtherComp,
                           int32 OtherBodyIndex,
                           bool bFromSweep,
                           const FHitResult& SweepResult);
    
    UFUNCTION(BlueprintCallable)
    void SetStunned(bool Stunned);
    
    UFUNCTION()
    void CombatRangeOverlap(
                            UPrimitiveComponent* OverlappedComponent,
                            AActor* OtherActor,
                            UPrimitiveComponent* OtherComp,
                            int32 OtherBodyIndex,
                            bool bFromSweep,
                            const FHitResult& SweepResult);

    UFUNCTION()
    void CombatRangeEndOverlap(
                               UPrimitiveComponent* OverlappedComponent,
                               AActor* OtherActor,
                               UPrimitiveComponent* OtherComp,
                               int32 OtherBodyIndex);
    
    UFUNCTION(BlueprintCallable)
    void PlayAttackMontage(FName Section, float PlayRate);
    
    UFUNCTION(BlueprintPure)
    FName GetAttackSectionName();
    
    UFUNCTION()
    void OnLeftWeaponOverlap(UPrimitiveComponent* OverLappedComponent,
                             AActor* OtherActor,
                             UPrimitiveComponent* OtherComp,
                             int32 OtherBodyIndex,
                             bool bFromSweep,
                             const FHitResult& SweepResult);
    UFUNCTION()
    void OnRightWeaponOverlap(UPrimitiveComponent* OverLappedComponent,
                              AActor* OtherActor,
                              UPrimitiveComponent* OtherComp,
                              int32 OtherBodyIndex,
                              bool bFromSweep,
                              const FHitResult& SweepResult);
    
    // 무기 콜리전박스에 대한 활성화/ 비활성화 함수
    UFUNCTION(BlueprintCallable)
    void ActivateLeftWeapon();
    UFUNCTION(BlueprintCallable)
    void DeactivateLeftWeapon();
    UFUNCTION(BlueprintCallable)
    void ActivateRightWeapon();
    UFUNCTION(BlueprintCallable)
    void DeactivateRightWeapon();
    
    // Victim : 번역 피해자라는 뜻
    void DoDamage(class AArcherCharacter* Victim);
    void SpawnBlood(AArcherCharacter* Victim, FName SocketName);
    
    // 캐릭터 기절 시도
    void StunCharacter(AArcherCharacter* Victim);
    
    void ResetCanAttack();
    
    UFUNCTION(BlueprintCallable)
    void FinishDeath();
    
    UFUNCTION()
    void DestroyEnemy();
    
private:
    
    // 영웅의 화살을 맞으면 파티클 생성
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class UParticleSystem* ImpactParticles;
    
    // 영웅의 화살을 맞으면 소리 생성
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class USoundCue* ImpactSound;
    
    // 현재 적의 채력
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float Health;
    
    // 현재 적의 최대 채력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float MaxHealth;
    
    // 적의 머리 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    FString HeadBone;
    
    // 적이 데미지를 입었을때 채력바를 표시할 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float HealthBarDisplayTime;
    
    FTimerHandle HealthBarTimer;
    
    // 데미지를 맞으면 몬타지 실행
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UAnimMontage* HitMontage;
    
    FTimerHandle HitReactTimer;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float HitReactTimeMin;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float HitReactTimeMax;
    
    bool bCanHitReact;
    
    // HitNumber 위젯과 히트한 위치를 저장하기 윈한 TMap  배열
    UPROPERTY(VisibleAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
    TMap<UUserWidget*, FVector> HitNumbers;
    
    // 히트넘버가 화면에서 제거되기까지의 시간
    UPROPERTY(EditAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float HitNumberDestroyTime;
    
    // AI 캐릭터를 위한 Behaviortree
    UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
    class UBehaviorTree* BehaviorTree;
    
    // 적이 이동할 지점
    UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
    FVector PatrolPoint;
    // 적이 이동할 지점2
    UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
    FVector PatrolPoint2;
    
    class AEnemyController* EnemyController;
    
    // 적이 적대적이 될 때를 위한 중첩 구체
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class USphereComponent* AgroSphere;
    
    // Hit 에니메이션을 재생할 때 true
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bStunned;
    
    // 기절 확율: 0 일때 기절하지 않고 1일때 기절한다
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float StunChance;
    
    // 공격 범윙에 있을 때 true 공격할 시간
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bInAttackRange;
    
    // 공격 범위의 sphere
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    USphereComponent* CombatRangeSphere;
    
    // 적의 다양하게 공격하는 몬타지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UAnimMontage* AttackMontage;
    
    // 4 가지의 공격 몬타지 섹션 이름들
    FName AttackLFast;
    FName AttackRFast;
    FName AttackL;
    FName AttackR;
    
    // 적의 왼쪽 무기 콜리전
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class UBoxComponent* LeftWeaponCollision;
    // 적의 오른쪽 무기 콜리전
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UBoxComponent* RightWeaponCollision;
    
    // 적의 기본 공격력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float BaseDamage;
    
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    FName LeftWeaponSocket;
    
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    FName RightWeaponSocket;
    
    // 적이 공격할 수 있는 경우에 ture 값
    UPROPERTY(VisibleAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
    bool bCanAttack;
    
    FTimerHandle AttackWaitTimer;
    
    // 공격 사이의 최소 대기 시간
    UPROPERTY(EditAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float AttackWaitTime;
    
    // 적의 죽는 모션
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UAnimMontage* DeathMontage;
    
    bool bDying;
    
    FTimerHandle DeathTimer;
    
    // 적이 죽고 사라지는 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    float DeathTime;
    
    

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    
    // 화살 명중 구현
    virtual void ArrowHit_Implementation(FHitResult HitResult, AActor* Archer, AController* ArcherController) override;
    
    // TakeDamage는 액터 클랙스에서 상속받는 기능이고  액터에 내장되어 있는 피해를 입는 기능이 있어 따로 기능을 만들 필요가 없다
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
    
    FORCEINLINE FString GetHeadBone() const {return HeadBone;}
    
    UFUNCTION(BlueprintImplementableEvent)
    void ShowHitNumber(int32 Damage, FVector HitLocation, bool bHeadShot);
    
    FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree;}

    
    
};
