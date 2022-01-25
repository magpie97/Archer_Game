// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "ArcherCharacter.h"
#include "Components/Capsulecomponent.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"


// Sets default values
AEnemy::AEnemy() :
Health(100.f),
MaxHealth(100.f),
HealthBarDisplayTime(4.f),
HitReactTimeMin(0.5f),
HitReactTimeMax(3.f),
bCanHitReact(true),
HitNumberDestroyTime(1.5f),
bStunned(false),
StunChance(0.5f),
AttackLFast(TEXT("AttackLFast")),
AttackRFast(TEXT("AttackRFast")),
AttackL(TEXT("AttackL")),
AttackR(TEXT("AttackR")),
BaseDamage(20.f),
LeftWeaponSocket(TEXT("FX_Trail_L_01")),
RightWeaponSocket(TEXT("FX_Trail_R_01")),
bCanAttack(true),
AttackWaitTime(1.f),
bDying(false),
DeathTime(.5f)

{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // USphereComponent를 사용하기 위해선 #include "Components/SphereComponent.h" 인클루드한다
    // Agro Sphere를 만든다
    AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
    AgroSphere->SetupAttachment(GetRootComponent());
    
    // 공격범위 sphere를 만든다
    CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRangeSphere"));
    CombatRangeSphere->SetupAttachment(GetRootComponent());
    
    // 왼쪽 오른쪽 무기 충돌 상자 생성
    LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Left Weapon Box"));
    LeftWeaponCollision->SetupAttachment(GetMesh(), FName("LeftWeaponBone"));
    RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Right Weapon Box"));
    RightWeaponCollision->SetupAttachment(GetMesh(), FName("RightWeaponBone"));
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
    
    AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlap);
    
    CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatRangeOverlap);
    CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatRangeEndOverlap);
    
    // 무기 상자에 대한 중첩 이벤트에 함수 바인딩
    LeftWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnLeftWeaponOverlap);
    RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightWeaponOverlap);
    
    // 무기 상자에 대한 충돌 사전 설정 set
    LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LeftWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    LeftWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    LeftWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RightWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    RightWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    RightWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    
    
    
    GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
    
    // mesh 및 capsule에 대한 카메라를 통과하게 만들어 적의 캡슐이나 매쉬에 캐릭터의 카메라가 부딫치는걸 방지
    GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    
    // AI controller를 가져온다
    EnemyController = Cast<AEnemyController>(GetController());
    
    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);
    }
    
    const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);
    
    const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);
    
    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatrolPoint);
        EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatrolPoint2);
        
        EnemyController->RunBehaviorTree(BehaviorTree);
    }
    
    
	
}

void AEnemy::ShowHealthBar_Implementation()
{
    GetWorldTimerManager().ClearTimer(HealthBarTimer);
    GetWorldTimerManager().SetTimer(HealthBarTimer, this, &AEnemy::HideHealthBar, HealthBarDisplayTime);
}

void AEnemy::Die()
{
    if (bDying) return;
    bDying = true;
    
    HideHealthBar();
    
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && DeathMontage)
    {
        AnimInstance->Montage_Play(DeathMontage);
    }
    
    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
        EnemyController->StopMovement();
    }
}

void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{
    if (bCanHitReact)
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(HitMontage, PlayRate);
            AnimInstance->Montage_JumpToSection(Section, HitMontage);
        }
        
        bCanHitReact = false;
        const float HitReactTime{FMath::FRandRange(HitReactTimeMin, HitReactTimeMax)};
        GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer, HitReactTime);
    }
    
}

void AEnemy::ResetHitReactTimer()
{
    bCanHitReact = true;
}

void AEnemy::StoreHitNumber(UUserWidget* HitNumber, FVector Location)
{
    HitNumbers.Add(HitNumber, Location);
    
    FTimerHandle HitNumberTimer;
    FTimerDelegate HitNumberDelegate;
    HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumber);
    GetWorld()->GetTimerManager().SetTimer(HitNumberTimer, HitNumberDelegate, HitNumberDestroyTime, false);
}

// 콜백함수
void AEnemy::DestroyHitNumber(UUserWidget* HitNumber)
{
    HitNumbers.Remove(HitNumber);
    HitNumber->RemoveFromParent();
}

void AEnemy::UpdateHitNumbers()
{
    for (auto& HitPair : HitNumbers)
    {
        UUserWidget* HitNumber{HitPair.Key};
        const FVector Location{HitPair.Value};
        FVector2D ScreenPosition;
        UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), Location, ScreenPosition);
        HitNumber->SetPositionInViewport(ScreenPosition);
    }
}

void AEnemy::AgroSphereOverlap(UPrimitiveComponent* OverLappedComponent,
                       AActor* OtherActor,
                       UPrimitiveComponent* OtherComp,
                       int32 OtherBodyIndex,
                       bool bFromSweep,
                       const FHitResult& SweepResult)
{
    if (OtherActor == nullptr) return;
    auto Character = Cast<AArcherCharacter>(OtherActor);
    if (Character)
    {
        if (EnemyController)
        {
            if (EnemyController->GetBlackboardComponent())
            {
                // 대상의 블랙보드의 키값을 설정한다 에셋에서만든 Target이 되겠다
                EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Character);
                
            }
        }
    }
    
    
}

void AEnemy::SetStunned(bool Stunned)
{
    bStunned = Stunned;
    
    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), Stunned);
    }
}

void AEnemy::CombatRangeOverlap(UPrimitiveComponent* OverLappedComponent,
                                AActor* OtherActor,
                                UPrimitiveComponent* OtherComp,
                                int32 OtherBodyIndex,
                                bool bFromSweep,
                                const FHitResult& SweepResult)
{
    if(OtherActor == nullptr) return;
    auto ArcherCharacter = Cast<AArcherCharacter>(OtherActor);
    if (ArcherCharacter)
    {
        bInAttackRange = true;
        if (EnemyController)
        {
            EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), true);
        }
    }

}

void AEnemy::CombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent,
                                   AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp,
                                   int32 OtherBodyIndex)
{
    if(OtherActor == nullptr) return;
    auto ArcherCharacter = Cast<AArcherCharacter>(OtherActor);
    if (ArcherCharacter)
    {
        bInAttackRange = false;
        if (EnemyController)
        {
            EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), false);
        }
    }
    
}

void AEnemy::PlayAttackMontage(FName Section, float PlayRate)
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && AttackMontage)
    {
        AnimInstance->Montage_Play(AttackMontage);
        AnimInstance->Montage_JumpToSection(Section, AttackMontage);
    }
    bCanAttack = false;
    GetWorldTimerManager().SetTimer(AttackWaitTimer, this, &AEnemy::ResetCanAttack, AttackWaitTime);
    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), false);
    }
}

FName AEnemy::GetAttackSectionName()
{
    FName SectionName;
    const int32 Section{ FMath::RandRange(1, 4) };
    switch (Section)
    {
    case 1:
        SectionName = AttackLFast;
        break;
    case 2:
        SectionName = AttackRFast;
        break;
    case 3:
        SectionName = AttackL;
        break;
    case 4:
        SectionName = AttackR;
        break;
    }
    return SectionName;
}

// 296
void AEnemy::DoDamage(AArcherCharacter* Victim)
{
    if (Victim == nullptr) return;
    UGameplayStatics::ApplyDamage(Victim, BaseDamage, EnemyController, this, UDamageType::StaticClass());

    
    if (Victim->GetMeleeImpactSound())
    {
        UGameplayStatics::PlaySoundAtLocation(this, Victim->GetMeleeImpactSound(), GetActorLocation());
    }
}

void AEnemy::StunCharacter(AArcherCharacter* Victim)
{
    if (Victim)
    {
        const float Stun{FMath::FRandRange(0.f, 1.f)};
        if (Stun <= Victim->GetStunChance())
        {
            Victim->Stun();
        }
    }
}

void AEnemy::ResetCanAttack()
{
    bCanAttack = true;
    
    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);
    }
}

void AEnemy::FinishDeath()
{
    GetMesh()->bPauseAnims = true;
    
    GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::DestroyEnemy, DeathTime);
}

void AEnemy::DestroyEnemy()
{
    Destroy();
}

void AEnemy::OnLeftWeaponOverlap(UPrimitiveComponent* OverLappedComponent,
                         AActor* OtherActor,
                         UPrimitiveComponent* OtherComp,
                         int32 OtherBodyIndex,
                         bool bFromSweep,
                         const FHitResult& SweepResult)
{
    auto Character = Cast<AArcherCharacter>(OtherActor);
    if (Character)
    {
        DoDamage(Character);
        
        SpawnBlood(Character, LeftWeaponSocket);
        StunCharacter(Character);
    }
}


void AEnemy::OnRightWeaponOverlap(UPrimitiveComponent* OverLappedComponent,
                          AActor* OtherActor,
                          UPrimitiveComponent* OtherComp,
                          int32 OtherBodyIndex,
                          bool bFromSweep,
                          const FHitResult& SweepResult)
{
    auto Character = Cast<AArcherCharacter>(OtherActor);
    if (Character)
    {
        DoDamage(Character);
        
        SpawnBlood(Character, RightWeaponSocket);
        StunCharacter(Character);
    }
}





void AEnemy::ActivateLeftWeapon()
{
    LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateLeftWeapon()
{
    LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightWeapon()
{
    RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateRightWeapon()
{
    RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AEnemy::SpawnBlood(AArcherCharacter* Victim, FName SocketName)
{
    const USkeletalMeshSocket* TipSocket{GetMesh()->GetSocketByName(SocketName)};
    if (TipSocket)
    {
        const FTransform SocketTransform{TipSocket->GetSocketTransform(GetMesh())};
        if (Victim->GetBloodParticles())
        {
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Victim->GetBloodParticles(), SocketTransform);
        }
    }
}




// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
    UpdateHitNumbers();

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


void AEnemy::ArrowHit_Implementation(FHitResult HitResult, AActor* Archer, AController* ArcherController)
{
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
    }
    if (ImpactParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location, FRotator(0.f), true);
    }

    
}


float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // 타겟의 블랙보드의 키를 캐릭터에 맞게 설정한다
    if (EnemyController)
    {
        EnemyController->GetBlackboardComponent()->SetValueAsObject(FName("Target"), DamageCauser);
    }
    
    if (Health - DamageAmount <= 0.f)
    {
        Health = 0.f;
        Die();
    }
    else
    {
        Health -= DamageAmount;
    }
    
    if (bDying) return DamageAmount;

    ShowHealthBar();
    // 화살 명중이 기절하는지 여부 랜덤으로 결정
    const float Stunned = FMath::FRandRange(0.f, 1.f);
    if (Stunned <= StunChance)
    {
        // 적이 스턴에 걸린다
        PlayHitMontage(FName("HitReactFront"));
        SetStunned(true);
    }
    
    return DamageAmount;
}
