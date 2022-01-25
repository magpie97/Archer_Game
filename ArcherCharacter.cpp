// Fill out your copyright notice in the Description page of Project Settings.


#include "ArcherCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h" // LineTraceSingleByChannel를 디버그 할때 사용한다
#include "Particles/ParticleSystemComponent.h"
#include "Item.h"
#include "Components/WidgetComponent.h"
#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Arrow.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Archer.h"
#include "ArrowHitInterface.h"
#include "Enemy.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Camera/CameraShake.h"



// Sets default values
AArcherCharacter::AArcherCharacter() :
// 회전과 바라보는 기본 감도
BaseTurnRate(45.f),
BaseLookUpRate(45.f),
// 조준했을때와 안했을때의 마우스 감도 값
HipTurnRate(90.f),
HipLookUpRate(90.f),
AimingTurnRate(20.f),
AimingLookUpRate(20.f),
// 마우스 감도의 크기 값
MouseHipTurnRate(1.f),
MouseHipLookUpRate(1.f),
MouseAimingTurnRate(0.6f),
MouseAimingLookUpRate(0.6f),
// 무기를 조준할때 값은 true
bAiming(false),
// 카메라 시야 값
CameraDefaultFOV(0.f), // beginplay에 설정했음
CameraZoomedFOV(40.f),
CameraCurrentFOV(0.f),
ZoomInterpSpeed(20.f),
// 크로스헤어 분산
CrosshairSpreadMultiplier(0.f),
CrosshairVelocityFactor(0.f),
CrosshairInAirFactor(0.f),
CrosshairAimFactor(0.f),
CrosshairShootingFactor(0.f),
// 활 자동 발사 값
bFireButtonPressed(false),
bShouldFire(true),
// 아이템 trace 변수값들
bShouldTraceForItems(false),
// 카메라 보간 위치 변수들
CameraInterpDistance(400.f), // 150
CameraInterpElevation(30.f), // 60
StartingHolyArrow(16),
StartingIceArrow(5),
StartingNatureArrow(5),
CombatState(ECombatState::ECS_Unoccupied),
bCrouching(false),
BaseMovementSpeed(650.f),
CrouchMovementSpeed(300.f),
StandingCapsuleHalfHeight(88.f), // 아처 캐릭터의 캡슐 컴포넌트의 기본 높이값이 88.f 이다
CrouchingCapsuleHalfHeight(44.f),    // 그래서 이 변수에 웅크리고 있을때의 절반값인 44.f를 넣어준다
BaseGroundFriction(2.f),
CrouchingGroundFriction(100.f),
bAimingButtonPressed(false),
// 화살을 얻을때 사운드 타이머 속성
bShouldPlayPickupSound(true),
bShouldPlayEquipSound(true),
PickupSoundResetTime(0.2f),
EquipSoundResetTime(0.2f),
// 애니메이션 아이콘 프로퍼티
HighlightedSlot(-1),
// 캐릭터 채력 
Health(100.f),
MaxHealth(100.f),
StunChance(.25f),
bDead(false)

{
     // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    
    //카메라 붐 만들기
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 300.f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->SocketOffset = FVector(0.f, 60.f, 110.f);
    CameraBoom->bEnableCameraLag = true; 
    CameraBoom->CameraLagSpeed = 20.f;
    
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // 이걸로 소켓을 만들어서 화살을 나가게 할수 있을꺼 같음
    FollowCamera->bUsePawnControlRotation = false;
    
    // 컨트롤러가 회전할 때 회전하지 않고 컨트롤러가 카메라에만 영향을 미치도록 한다.
    // #include "GameFramework/CharacterMovementComponent.h" 를 include 한다
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;
    
    // 캐릭터 이동 설정
    // 캐릭터는 이 회전 속도로 입력 방향으로 이동합니다.
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    GetCharacterMovement()->JumpZVelocity = 500.f;
    GetCharacterMovement()->AirControl = 0.7f; // 0.2default
    
    // 캐릭터 달리기 속도
    //SprintSpeedMultiplier = 2.0f;
    

}


// 공격받아 체력이 없으면 죽는 함수
float AArcherCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    if (Health - DamageAmount <= 0.f)
    {
        Health = 0.f;
        Die();
        
        auto EnemyController = Cast<AEnemyController>(EventInstigator);
        if (EnemyController)
        {
            EnemyController->GetBlackboardComponent()->SetValueAsBool(FName(TEXT("CharacterDead")), true);
        }
    }
    else
    {
        Health -= DamageAmount;
    }
    
    return DamageAmount;
}

void AArcherCharacter::Die()
{
    bDead = true;
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && DeathMontage)
    {
        AnimInstance->Montage_Play(DeathMontage);
        
        
    }
}

void AArcherCharacter::FinishDeath()
{
    GetMesh()->bPauseAnims = true;
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        DisableInput(PC);
    }
}



// Called when the game starts or when spawned
void AArcherCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (FollowCamera)
    {
        CameraDefaultFOV = GetFollowCamera()->FieldOfView;
        CameraCurrentFOV = CameraDefaultFOV;
    }
    // 스폰된 default 활무기와 부착된 활 매쉬를 장착한다
    EquipWeapon(SpawnDefaultWeapon());
    Inventory.Add(EquippedWeapon);
    EquippedWeapon->SetSlotIndex(0);
    EquippedWeapon->SetCharacter(this);
    
    InitializeArrowMap();
    GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
    
}




void AArcherCharacter::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation{Controller->GetControlRotation()};
        const FRotator YawRotation{0, Rotation.Yaw, 0};
        
        const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};
        AddMovementInput(Direction, Value);
    }
}

void AArcherCharacter::MoveRight(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation{Controller->GetControlRotation()};
        const FRotator YawRotation{0, Rotation.Yaw, 0};
        
        const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y)};
        AddMovementInput(Direction, Value);
    }
}

void AArcherCharacter::Sprint(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation{Controller->GetControlRotation()};
        const FRotator YawRotation{0.f, Rotation.Yaw, 0};
        
        const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};
        AddMovementInput(Direction, Value);
    }
}



void AArcherCharacter::TurnAtRate(float Rate) // 구조체로 값을 조절
{
    //에드컨트롤러 Yawinput 함수에 Rate값과 BaseTurnRate, getworld함수의 GetDeltaSeconds함수를 붙이고 다 곱한다
    // 상하 축을 기준으로 한 회전각 pinch
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AArcherCharacter::LookUpAtRate(float Rate)
{
    // 좌우 축을 기준으로 한 회전각 pitch
    AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}




// 에임 줌을 했냐 안했냐의 체크
void AArcherCharacter::Turn(float Value)
{
    float TurnScaleFactor{};
    if (bAiming)
    {
        TurnScaleFactor = MouseAimingTurnRate;
    }
    else
    {
        TurnScaleFactor = MouseHipTurnRate;
    }
    AddControllerYawInput(Value * TurnScaleFactor);
}

void AArcherCharacter::LookUp(float Value)
{
    float LookUpScaleFactor{};
    if (bAiming)
    {
        LookUpScaleFactor = MouseAimingLookUpRate;
    }
    else
    {
        LookUpScaleFactor = MouseHipLookUpRate;
    }
    AddControllerPitchInput(Value * LookUpScaleFactor);
}






void AArcherCharacter::FireWeapon()
{
    if (EquippedWeapon == nullptr) return;
    if (CombatState != ECombatState::ECS_Unoccupied) return;
    
    if (WeaponHasArrow())
    {
        PlayFireSound();
        SendArrow();
        PlayBowFireMontage();
        // 가지고 있는 활 무기에서 화살 1개를 빼기
        EquippedWeapon->DecrementArrow();
        
        StartFireTimer();
        
        
    }

    
    
    
    //여기 아래부터 화살 날라가는 코드 작성
    /**
    // 여기서 부터 새로운  projectile 코드
    //FVector CameraLocation; 아직 필요가 없을꺼 같다
    //FRotator CameraRotation; 이것도
    FVector CameraLocation;
    FRotator CameraRotation;
    //GetActorEyesViewPoint(CameraLocation, CameraRotation); // 이게 눈에서 화살이 나옴 내가 원하는 그림이 아님
    
     GetActorForwardVector(); // 이건 새로 추가한 함수
    
     FVector MuzzleLocation = CameraLocation + FTransform(CameraRotation).TransformVector(MuzzleOffset);
    FRotator MuzzleRotation = CameraRotation;
    MuzzleRotation.Pitch += 0.f;
    
    UWorld* World = GetWorld();
    if (World)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();
        
        AFPSProjectile* Projectile = World->SpawnActor<AFPSProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParams);
        if (Projectile)
        {
            FVector LaunchDirection = MuzzleRotation.Vector();
            Projectile->FireInDiraction(LaunchDirection);
        }
    }
    // -----------------------------
     */
    
    
}


bool AArcherCharacter::GetArrowBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)
{
    FVector OutArrowBeamLocation;
    // check for crosshair trace hit 크로스헤어 trace hit이 됐는지 체크
    FHitResult CrosshairHitResult;
    bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutArrowBeamLocation);
    
    if (bCrosshairHit)
    {
        // 임시 arrow beam 위치  여전히 bow에서 추적해야 함
        OutArrowBeamLocation = CrosshairHitResult.Location;
    }
    else // no crosshair trace hit
    {
        // OutArrowBeamLocation는 line trace의 End 위치이다
    }
    
    // 화살의 궤적이 벽을 뚥고 End에 파티클이 생성되는걸
    const FVector WeaponTraceStart{ MuzzleSocketLocation };
    const FVector StartToEnd{OutArrowBeamLocation - MuzzleSocketLocation };
    const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f}; // 1.25
    
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(this);
    
    // 화살에 중력을 넣는다 안될 수 있다
    GetWorld()->LineTraceSingleByChannel(OutHitResult, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
    
    if (!OutHitResult.bBlockingHit) // object between barrel and beamendpoint?
    {
        OutHitResult.Location = OutArrowBeamLocation;
        return false;
    }
    return true;
}


// 우클릭으로 조준했을때 bAiming이 ture 이거나 false로 설정한다
void AArcherCharacter::AimingButtonPressed()
{
    bAimingButtonPressed = true;
    if (CombatState != ECombatState::ECS_Reloading && CombatState != ECombatState::ECS_Equipping && CombatState != ECombatState::ECS_Stunned)
    {
        Aim();
    }
    
}
void AArcherCharacter::AimingButtonReleased()
{
    bAimingButtonPressed = false;
    StopAiming();

}



// 조준을 클릭 했을때
// 이 FInterpTo()함수를 사용하기위해선
// float Current, float Target, float DeltaTime, float InterpSpeed 가 필요하다
void AArcherCharacter::CameraInterpZoom(float DeltaTime)
{
    // 조준을 클릭 했을때
    if (bAiming)
    {
        // 이 FInterpTo()함수를 사용하기위해선
        // float Current, float Target, float DeltaTime, float InterpSpeed 가 필요하다
        CameraCurrentFOV = FMath::FInterpTo(
            CameraCurrentFOV,
            CameraZoomedFOV,
            DeltaTime,
            ZoomInterpSpeed);
    }
    else
    {
        // Interpolate to default FOV
        CameraCurrentFOV = FMath::FInterpTo(
            CameraCurrentFOV,
            CameraDefaultFOV,
            DeltaTime,
            ZoomInterpSpeed);
    }
    GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AArcherCharacter::SetLookRates()
{
    if (bAiming)
    {
        BaseTurnRate = AimingTurnRate;
        BaseLookUpRate = AimingLookUpRate;
    }
    else
    {
        BaseTurnRate = HipTurnRate;
        BaseLookUpRate = AimingLookUpRate;
    }
}


// 50
void AArcherCharacter::CalculateCrosshairSpread(float DeltaTime)
{
    FVector2D WalkSpeedRange{0.f, 600.f };
    FVector2D VelocityMultiplierRange{0.f, 1.f}; // default 0 1
    FVector Velocity{ GetVelocity() };
    Velocity.Z = 0.f; // default 0
    
    // GetMappedRangeValueClamped() 함수
    CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
    
    // 공중에 있을때 크로스헤어의 값을 계산한다
    if (GetCharacterMovement()->IsFalling()) // 공중에 있으면 실행
    {
        // 크로스헤어가 공중에서 천천히 분산
        CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 3.25f); // 2.25
    }
    else // 캐릭터가 땅에 떨어졌을때
    {
        // 땅에 있으면 크로스헤어가 빠르게 축소한다
        CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f); // 30
    }
    // 크로스헤어 조준중알때를 계산한다
    if (bAiming)
    {
        // 크로스헤어를 아주 빠르게 축소
        CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.5f, DeltaTime, 30.f); // default 0.5 / 30
    }
    else
    {
        // 크로스헤어를 아주 빠르게 정상으로 돌림
        CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.0f, DeltaTime, 30.f); // 0 / 30
    }
    
    CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor;
}



// 활 자동 발사 함수들------------------------
void AArcherCharacter::FireButtonPressed()
{
    if (WeaponHasArrow())
    {
        bFireButtonPressed = true;
        FireWeapon();
    }

}
/*
 // FireButtonpressed() 에 있는 원래 있던 코드  베럴소켓을 고치려만든 코드이다
 void AArcherCharacter::FireButtonPressed()
 {
     if (WeaponHasArrow())
     {
         bFireButtonPressed = true;
         if (WeaponHasArrow())
         {
             StartFireTimer();
         }
     }

 }
 */

void AArcherCharacter::FireButtonReleased()
{
    bFireButtonPressed = false;
}


void AArcherCharacter::StartFireTimer()
{
    if (EquippedWeapon == nullptr) return;
    CombatState = ECombatState::ECS_FireTimerInProgress;
    
    GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AArcherCharacter::AutoFireReset, EquippedWeapon->GetAutoFireRate());

}

void AArcherCharacter::AutoFireReset()
{
    if (CombatState == ECombatState::ECS_Stunned) return;
    
    CombatState = ECombatState::ECS_Unoccupied;
    
    if (WeaponHasArrow())
    {
        if (bFireButtonPressed)
        {
            FireWeapon();
            
        }
    }
    else
    {
        // 화살을 재장전한다
        ReloadWeapon();
    }
}
//-------------------------------------


bool AArcherCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
    // 뷰포트 크기를 가져오기
    FVector2D ViewportSize;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
    }
    
    FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
    FVector CrosshairWorldPosition;
    FVector CrosshairWorldDirection;
    
    //DeprojectScreenToWorld는 주어진 2d 화면 공간 좌표를 3D 세계 공간 점과 방향으로 변환할때 사용한다
    bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
                                                                   CrosshairLocation,
                                                                   CrosshairWorldPosition,
                                                                   CrosshairWorldDirection);
    
    if (bScreenToWorld)
    {
        // world location 바깥쪽의 크로스헤어를 추적
        const FVector Start{CrosshairWorldPosition};
        const FVector End{Start + CrosshairWorldDirection * 500'000.f};
        OutHitLocation = End;
        GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);
        if (OutHitResult.bBlockingHit)
        {
            OutHitLocation = OutHitResult.Location;
            return true;
        }
    }
    return false;
}


void AArcherCharacter::TraceForitems()
{
    if (bShouldTraceForItems)
    {
        FHitResult ItemTraceResult;
        FVector HitLocation;
        TraceUnderCrosshairs(ItemTraceResult, HitLocation);
        if (ItemTraceResult.bBlockingHit)
        {
            TraceHitItem = Cast<AItem>(ItemTraceResult.Actor);
            const auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
            if (TraceHitWeapon)
            {
                if (HighlightedSlot == -1)
                {
                    // 현재 슬롯을 강조 표시를 하지 않는다
                    HighlightInventorySlot();
                }
            }
            else
            {
                // Is a slot being highlight;
                if (HighlightedSlot != -1)
                {
                    // UnHighlight the slot
                    UnHighlightInventorySlot();
                }
            }
            
            if (TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
            {
                TraceHitItem = nullptr;
            }
            
            if (TraceHitItem && TraceHitItem->GetPickupWidget())
            {
                // 아이템의 pickupwidget 표시
                TraceHitItem->GetPickupWidget()->SetVisibility(true);
                
                // 인벤토리가 꽉차면 줍기 텍스트를 바꾸기로 바꾸기
                if (Inventory.Num() >= INVENTORY_CAPACITY)
                {
                    // 인벤토리가 꽉참
                    TraceHitItem->SetCharacterInventoryFull(true);
                }
                else
                {
                    // 인벤토리가 남아있다면
                    TraceHitItem->SetCharacterInventoryFull(false);
                }
            }
            
            // 마지막에 프레임에 AItem을 맞춤
            if (TraceHitItemLastFrame)
            {
                if (TraceHitItem != TraceHitItemLastFrame)
                {
                    // we are hitting a diffenet Aitem this frame from last frame
                    // of Aitem is null
                    TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
                }
            }
            
            // store a reference to hititem for next frame
            TraceHitItemLastFrame = TraceHitItem;
        }
    }
    else if (TraceHitItemLastFrame)
    {
        // no longer overlapping any items item last frame should not show widget
        TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
    }
}


// TSubclassOf 변수를 체크한다
AWeapon* AArcherCharacter::SpawnDefaultWeapon()
{
    if (DefaultWeaponClass)
    {
        // 활 무기를 스폰한다
        return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
    }
    return nullptr;
}

void AArcherCharacter::EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping)
{
    if (WeaponToEquip)
    {
        // 활 무기를 스폰한다
        const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("LeftHandSocket"));
        if (HandSocket)
        {
            // HandSocket에 LeftHandSocket부착한다  부착한다
            HandSocket->AttachActor(WeaponToEquip, GetMesh());
        }
        
        if (EquippedWeapon == nullptr)
        {
            // -1 == no EquippedWeapon yet
            //
            EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
        }
        else if (!bSwapping)
        {
            EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
        }
        
        // 새롭게 스폰된 활 무기 EquippedWeapon를 설정한다
        EquippedWeapon = WeaponToEquip;
        EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
    }
}


void AArcherCharacter::DropWeapon()
{
    if (EquippedWeapon)
    {
        // 이게 어디에 쓰이는지 공부하자
        FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
        EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
        
        EquippedWeapon->SetItemState(EItemState::EIS_Falling);
        EquippedWeapon->ThrowWeapon();
    }
}


void AArcherCharacter::SelectButtonPressed()
{
    if (CombatState != ECombatState::ECS_Unoccupied) return;
    if (TraceHitItem)
    {
        TraceHitItem->StartItemCurve(this, true);
        TraceHitItem = nullptr;

    }
    
}

void AArcherCharacter::SelectButtonReleased()
{
    
}


void AArcherCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
    if (Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex())
    {
        Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
        WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
    }
    
    DropWeapon();
    EquipWeapon(WeaponToSwap, true);
    TraceHitItem = nullptr;
    TraceHitItemLastFrame = nullptr;
}



void AArcherCharacter::InitializeArrowMap()
{
    ArrowMap.Add(EArrowType::EAT_HolyArrow, StartingHolyArrow);
    ArrowMap.Add(EArrowType::EAT_IceArrow, StartingIceArrow);
    ArrowMap.Add(EArrowType::EAT_NatureArrow, StartingNatureArrow);
}


bool AArcherCharacter::WeaponHasArrow()
{
    if (EquippedWeapon == nullptr) return false;
    
    return EquippedWeapon->GetArrow() > 0;
}






void AArcherCharacter::PlayFireSound()
{
    //활 쏘는 소리
    if (EquippedWeapon->GetFireSound())
    {
        // #include "Kismet/GameplayStatics.h" 인클루드 하고 사용한다 26번
        // #include "Sound/SoundCue.h" 도 인클루드 한다
        UGameplayStatics::PlaySound2D(this, EquippedWeapon->GetFireSound());
    }
}

void AArcherCharacter::SendArrow()
{
    // 가지고 있는 화살을 재장전할때 보낸다
    const USkeletalMeshSocket* BowBarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BowEffect");// 95 활나오는곳 위치 조정
    if (BowBarrelSocket)
    {
        // #include "Engine/SkeletalMeshSocket.h" 를 인클루드 한다
        const FTransform SocketTransform = BowBarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
        
        if (EquippedWeapon->GetBowEffect())
        {
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EquippedWeapon->GetBowEffect(), SocketTransform);
        }
        
        FHitResult ArrowBeamHitResult;
        bool bArrowBeamEnd = GetArrowBeamEndLocation(SocketTransform.GetLocation(), ArrowBeamHitResult);
        if (bArrowBeamEnd)
        {
            // Hit Actor가 ArrowHitInterface를 구현한다면
            if (ArrowBeamHitResult.Actor.IsValid())
            {
                IArrowHitInterface* ArrowHitInterface = Cast<IArrowHitInterface>(ArrowBeamHitResult.Actor.Get());
                if (ArrowHitInterface)
                {
                    ArrowHitInterface->ArrowHit_Implementation(ArrowBeamHitResult, this, GetController());
                }
                
                AEnemy* HitEnemy = Cast<AEnemy>(ArrowBeamHitResult.Actor.Get());
                if (HitEnemy)
                {
                    int32 Damage{};
                    if (ArrowBeamHitResult.BoneName.ToString() == HitEnemy->GetHeadBone())
                    {
                        // 머리에 맞았을때
                        Damage = EquippedWeapon->GetHeadShotDamage(); // 262 450
                        UGameplayStatics::ApplyDamage(
                                                      ArrowBeamHitResult.Actor.Get(),
                                                      Damage,
                                                      GetController(),
                                                      this,
                                                      UDamageType::StaticClass());
                        HitEnemy->ShowHitNumber(Damage, ArrowBeamHitResult.Location, true);
                    }
                    else
                    {
                        // 몸통에 맞았을때
                        Damage = EquippedWeapon->GetDamage();
                        UGameplayStatics::ApplyDamage(
                                                      ArrowBeamHitResult.Actor.Get(),
                                                      Damage,
                                                      GetController(),
                                                      this,
                                                      UDamageType::StaticClass());
                        HitEnemy->ShowHitNumber(Damage, ArrowBeamHitResult.Location, false);
                    }
                    
                    
                }
                
            }
            else
            {
                // 디폴트 파티클을 스폰한다
                if (ImpactParticles)
                {
                    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, ArrowBeamHitResult.Location);
                }
            }
            

            UParticleSystemComponent* ArrowBeam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ArrowBeamParticles, SocketTransform);
            if (ArrowBeam)
            {
                ArrowBeam->SetVectorParameter(FName("Target"), ArrowBeamHitResult.Location);
            }
        }
    }
}

void AArcherCharacter::PlayBowFireMontage()
{
    // 활을 쏠때 시작되는 HipFireMontage 시퀀스
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && HipFireMontage)
    {
        AnimInstance->Montage_Play(HipFireMontage);         // 몬타지를 만들고 넣는다
        AnimInstance->Montage_JumpToSection(FName("StartFire"));
    }
}

void AArcherCharacter::ReloadButtonPressed()
{
    ReloadWeapon();
}

void AArcherCharacter::ReloadWeapon()
{
    if (CombatState != ECombatState::ECS_Unoccupied) return;
    

    
    if (EquippedWeapon == nullptr) return;
    
    // 현재 유형에맞는 화살이 있을경우
    if (CarryingArrow() && !EquippedWeapon->ArrowCaseIsFull())
    {
        if (bAiming)
        {
            StopAiming();
        }
        
        CombatState = ECombatState::ECS_Reloading;
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance && ReloadMontage)
        {
            AnimInstance->Montage_Play(ReloadMontage);
            AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
        }
    }
}



bool AArcherCharacter::CarryingArrow()
{
    if (EquippedWeapon == nullptr) return false;
    
    auto ArrowType = EquippedWeapon->GetArrowType();
    
    if (ArrowMap.Contains(ArrowType))
    {
        return ArrowMap[ArrowType] > 0;
    }
    
    return false;
}


void AArcherCharacter::CrouchButtonPressed()
{
    if (!GetCharacterMovement()->IsFalling())
    {
        bCrouching = !bCrouching;
    }
    if (bCrouching)
    {
        GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
        GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
        
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
        GetCharacterMovement()->GroundFriction = BaseGroundFriction;
    }
}

void AArcherCharacter::Jump()
{
    if (bCrouching)
    {
        bCrouching = false;
        GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
    }
    else
    {
        ACharacter::Jump();
    }
}


void AArcherCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
    float TargetCapsuleHalfHeight{};
    if (bCrouching)
    {
        TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;
    }
    else
    {
        TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;
    }
    // 여기서 캐릭터의 캡슐 컴포넌트를 사용하기 위해선 헤더를 인클루드 해야한다
    // #include "Components/CapsuleComponent.h"
    // 아처캐릭터cpp 에서 기본값을 설정했기에 캡슐 컴포넌트에 Get함수를 사용한다 인터프 속도는 15.f 으로 했다
    const float InterpHalfHeight{FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHeight, DeltaTime, 15.f)};
    
    // 웅크리고 있을때는 음수의 값으로 서있을때의 양수의 값으로
    const float DeltaCapsuleHalfHeight{InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()};
    const FVector MeshOffset{0.f, 0.f, -DeltaCapsuleHalfHeight};
    GetMesh()->AddLocalOffset(MeshOffset);
    
    GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}


void AArcherCharacter::Aim()
{
    bAiming = true;
    
    // 활을 줌할때 나오는 소리
    if (ChargingSound)
    {
        UGameplayStatics::PlaySound2D(this, ChargingSound);
    }
    
    // SetFieldOfView() 함수는 카메라 컴포넌트의 함수이며 float으로 사용함
    // 조준할때 속도가 웅크리고 있을때의 속도와 같게 함
    GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
    

}

void AArcherCharacter::StopAiming()
{
    bAiming = false;
    

    
    // 웅크리고 있지 않을때 캐릭터의 속도는 기본속도로 함
    if (!bCrouching)
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
    }
}

void AArcherCharacter::PickupArrow(class AArrow* Arrow)
{
    // ArrowMap에 ArrowType들이 포함되어있는지 if문으로 확인한다
    if (ArrowMap.Find(Arrow->GetArrowType()))
    {
        // ArrowMap에서 화살의 종류들을 가져온다
        int32 ArrowCount{ArrowMap[Arrow->GetArrowType()]};
        ArrowCount += Arrow->GetItemCount();
        // 이 타입의 Map에서 화살의 합을 설정한다
        ArrowMap[Arrow->GetArrowType()] = ArrowCount;
    }
    
    if (EquippedWeapon->GetArrowType() == Arrow->GetArrowType())
    {
        // 만약 화살통이나 장전한 화살도 없다면 재장전을 한다
        if (EquippedWeapon->GetArrow() == 0 )
        {
            ReloadWeapon();
        }
    }
    
    Arrow->Destroy();
}

void AArcherCharacter::FKeyPressed()
{
    if (EquippedWeapon->GetSlotIndex() == 0) return;
    ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
    
}

void AArcherCharacter::OneKeyPressed()
{
    if (EquippedWeapon->GetSlotIndex() == 1) return;
    ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);

}

void AArcherCharacter::TwoKeyPressed()
{
    if (EquippedWeapon->GetSlotIndex() == 2) return;
    ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
    
}


void AArcherCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
    
    const bool bCanExchangeItems =
            (CurrentItemIndex != NewItemIndex) &&
            (NewItemIndex < Inventory.Num()) &&
            (CombatState == ECombatState::ECS_Unoccupied || CombatState == ECombatState::ECS_Equipping);
    
    if (bCanExchangeItems)
    {
        if (bAiming)
        {
            StopAiming();
        }
        
        auto OldEquippedWeapon = EquippedWeapon;
        auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);
        EquipWeapon(NewWeapon);
        
        OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
        NewWeapon->SetItemState(EItemState::EIS_Equipped);
        
        CombatState = ECombatState::ECS_Equipping;
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance && EquipMontage)
        {
            AnimInstance->Montage_Play(EquipMontage, 1.0f);
            AnimInstance->Montage_JumpToSection(FName("Equip"));
        }
        NewWeapon->PlayEquipSound(true);
    }

}

int32 AArcherCharacter::GetEmptyInventorySlot()
{
    for (int32 i = 0; i < Inventory.Num(); i++)
    {
        if (Inventory[i] == nullptr)
        {
            return i;
        }
    }
    if (Inventory.Num() < INVENTORY_CAPACITY)
    {
        return Inventory.Num();
    }
    
    return -1; // 인벤토리가 꽉 참
}

void AArcherCharacter::HighlightInventorySlot()
{
    const int32 EmptySlot{GetEmptyInventorySlot()};
    HighlightIconDelegate.Broadcast(EmptySlot, true);
    HighlightedSlot = EmptySlot;
}

void AArcherCharacter::UnHighlightInventorySlot()
{
    HighlightIconDelegate.Broadcast(HighlightedSlot, false);
    HighlightedSlot = -1;
}

void AArcherCharacter::Stun()
{
    if (Health <= 0.f) return;
    
    CombatState = ECombatState::ECS_Stunned;
    
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && HitReactMontage)
    {
        AnimInstance->Montage_Play(HitReactMontage);
    }
}

// 현재 캐릭터가 땅을 밟고 있을때 어떤 매쉬가 닿고 있는지 트레이싱 한다 
EPhysicalSurface AArcherCharacter::GetSurfaceType()
{
    FHitResult HitResult;
    const FVector Start{GetActorLocation()};
    const FVector End{Start + FVector(0.f, 0.f, -400.f)};
    FCollisionQueryParams QueryParams;
    QueryParams.bReturnPhysicalMaterial = true;
    
    GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);
    
    // PhysMaterial 를 사용하기 위해선
    // #include "PhysicalMaterials/PhysicalMaterial.h"를 인클루드한다
    // 이 UE_LOG는 확인차 만들었음
    /*
    auto HitSurface = HitResult.PhysMaterial->SurfaceType;
    if (HitSurface == EPS_Grass)
    {
        UE_LOG(LogTemp, Warning, TEXT("this is grass field"));
    }
    */
    
    return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
    
    
}


void AArcherCharacter::FinishEquipping()
{
    if (CombatState == ECombatState::ECS_Stunned) return;
    CombatState = ECombatState::ECS_Unoccupied;
    if (bAimingButtonPressed)
    {
        Aim();
    }
}

void AArcherCharacter::EndStun()
{
    CombatState = ECombatState::ECS_Unoccupied;
    
    if (bAimingButtonPressed)
    {
        Aim();
    }
}






// Called every frame
void AArcherCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 조준할 때 확대/축소를 위한 보간 처리
    CameraInterpZoom(DeltaTime);
    
    // 조준 했을때 감도 바꾸기
    SetLookRates();
    
    // 크로스헤어의 퍼짐을 계산한다
    CalculateCrosshairSpread(DeltaTime);
    
    // overlappedItemCount를 체크하고 아이템의 위치를 추적한다
    TraceForitems();
    
    // 웅크리거나 서 있을때의 따라 캐릭터의 캡슐의 절반 높이를 interp 한다
    InterpCapsuleHalfHeight(DeltaTime);


}

// Called to bind functionality to input
void AArcherCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    check(PlayerInputComponent);
    
    // wasd 키와 마우스 yaw, pitch
    PlayerInputComponent->BindAxis("MoveForward", this, &AArcherCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AArcherCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Sprint", this, &AArcherCharacter::Sprint);
    PlayerInputComponent->BindAxis("TurnRate", this, &AArcherCharacter::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUpRate", this, &AArcherCharacter::LookUpAtRate);
    PlayerInputComponent->BindAxis("Turn", this, &AArcherCharacter::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &AArcherCharacter::LookUp);
    
    // 점프
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AArcherCharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    
    //왼쪽 마우스로 화살 발사
    PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AArcherCharacter::FireButtonPressed);
    PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AArcherCharacter::FireButtonReleased);

    // 조준
    PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AArcherCharacter::AimingButtonPressed);
    PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AArcherCharacter::AimingButtonReleased);
    
    PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AArcherCharacter::SelectButtonPressed);
    PlayerInputComponent->BindAction("Select", IE_Released, this, &AArcherCharacter::SelectButtonReleased);
    
    PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AArcherCharacter::ReloadButtonPressed);
    
    //PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AArcherCharacter::Sprint);
    //PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AArcherCharacter::StopSprinting);
    
    PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AArcherCharacter::CrouchButtonPressed);
    
    PlayerInputComponent->BindAction("FKey", IE_Pressed, this, &AArcherCharacter::FKeyPressed);
    PlayerInputComponent->BindAction("1Key", IE_Pressed, this, &AArcherCharacter::OneKeyPressed);
    PlayerInputComponent->BindAction("2Key", IE_Pressed, this, &AArcherCharacter::TwoKeyPressed);





}

void AArcherCharacter::FinishReloading()
{
    if (CombatState == ECombatState::ECS_Stunned) return;
    
    // 컴뱃스테이트를 업데이트한다
    CombatState = ECombatState::ECS_Unoccupied;
    
    if (bAimingButtonPressed)
    {
        Aim();
    }
    
    if (EquippedWeapon == nullptr) return;
    
    const auto ArrowType{EquippedWeapon->GetArrowType()};
    
    // ArrowMap을 업데이트한다
    if (ArrowMap.Contains(EquippedWeapon->GetArrowType()))
    {
        // 캐릭터가 소지하고 있는 화살의 양과 종류
        int32 CarriedArrow = ArrowMap[ArrowType];
        
        // Space left in the arrow case of EquippedWeapon
        const int32 ArrowCaseEmptySpace = EquippedWeapon->GetArrowCaseCapacity() - EquippedWeapon->GetArrow();
        
        if (ArrowCaseEmptySpace > CarriedArrow)
        {
            // 가지고 있는 화살들을 화살통에 재장전한다
            EquippedWeapon->ReloadArrow(CarriedArrow);
            CarriedArrow = 0;
            ArrowMap.Add(ArrowType, CarriedArrow);
        }
        else
        {
            // fill the Arrowcase
            EquippedWeapon->ReloadArrow(ArrowCaseEmptySpace);
            CarriedArrow -= ArrowCaseEmptySpace;
            ArrowMap.Add(ArrowType, CarriedArrow);
        }
    }
}

void AArcherCharacter::ResetPickupSoundTimer()
{
    bShouldPlayPickupSound = true;
}
void AArcherCharacter::ResetEquipSoundTimer()
{
    bShouldPlayEquipSound = true;
}


float AArcherCharacter::GetCrosshairSpreadMultiplier() const
{
    return  CrosshairSpreadMultiplier;
}

void AArcherCharacter::IncrementOverlappedItemCount(int8 Amount)
{
    if (OverlappedItemCount + Amount <= 0)
    {
        OverlappedItemCount = 0;
        bShouldTraceForItems = false;
    }
    else
    {
        OverlappedItemCount += Amount;
        bShouldTraceForItems = true;
    }
}

FVector AArcherCharacter::GetCameraInterpLocation()
{
    const FVector CameraWorldLocation{FollowCamera->GetComponentLocation() };
    const FVector CameraForward{FollowCamera->GetForwardVector()};
    // desired = CameraWorldLocation + forward * A + Up * B
    return CameraWorldLocation + CameraForward * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);
}

void AArcherCharacter::GetPickupItem(AItem* Item)
{
    Item->PlayEquipSound();
    
    auto Weapon = Cast<AWeapon>(Item);
    if (Weapon)
    {
        if (Inventory.Num() < INVENTORY_CAPACITY)
        {
            Weapon->SetSlotIndex(Inventory.Num());
            Inventory.Add(Weapon);
            Weapon->SetItemState(EItemState::EIS_PickedUp);
        }
        else    // 인벤토리가 꽉 차면 무기를 바꾼다
        {
            SwapWeapon(Weapon);
        }
    }
    
    auto Arrow = Cast<AArrow>(Item);
    if (Arrow)
    {
        PickupArrow(Arrow);
    }
}

void AArcherCharacter::StartPickupSoundTimer()
{
    bShouldPlayPickupSound = false;
    GetWorldTimerManager().SetTimer(
        PickupSoundTimer,
        this,
        &AArcherCharacter::ResetPickupSoundTimer,
        PickupSoundResetTime);
}

void AArcherCharacter::StartEquipSoundTimer()
{
    bShouldPlayEquipSound = false;
    GetWorldTimerManager().SetTimer(
        PickupSoundTimer,
        this,
        &AArcherCharacter::ResetEquipSoundTimer,
        EquipSoundResetTime);
}

/*
// 캐릭터 달리기
void AArcherCharacter::Sprint()
{
    
    GetCharacterMovement()->MaxWalkSpeed *= SprintSpeedMultiplier;
}

void AArcherCharacter::StopSprinting()
{
    GetCharacterMovement()->MaxWalkSpeed /= SprintSpeedMultiplier;
}
*/
