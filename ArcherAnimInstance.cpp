// Fill out your copyright notice in the Description page of Project Settings.


#include "ArcherAnimInstance.h"
#include "ArcherCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon.h"
#include "WeaponType.h"

UArcherAnimInstance::UArcherAnimInstance() :
Speed(0),
bIsInAir(false),
bIsAccelerating(false),
MovementOffsetYaw(0.f),
LastMovementOffsetYaw(0.f),
bAiming(false),
TIPCharacterYaw(0.f),
TIPCharacterYawLastFrame(0.f),
RootYawOffset(0.f),
Pitch(0.f),
bReloading(false),
OffsetState(EOffsetState::EOS_Hip),
CharacterRotation(FRotator(0.f)),
CharacterRotationLastFrame(FRotator(0.f)),
YawDelta(0.f),
RecoilWeight(1.0f),
bTurningInPlace(false),
EquippedWeaponType(EWeaponType::EWT_MAX),
bShouldUseFABRIK(false)
{

}

void UArcherAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
    if (ArcherCharacter == nullptr)
    {
        ArcherCharacter = Cast<AArcherCharacter>(TryGetPawnOwner());
        

    }
    if (ArcherCharacter)
    {
        bCrouching = ArcherCharacter->GetCrouching();
        bReloading = ArcherCharacter->GetCombatState() == ECombatState::ECS_Reloading;
        bEquipping = ArcherCharacter->GetCombatState() == ECombatState::ECS_Equipping;
        bShouldUseFABRIK = ArcherCharacter->GetCombatState() == ECombatState::ECS_Equipping || ArcherCharacter->GetCombatState() == ECombatState::ECS_FireTimerInProgress;
        
        // Get the speed of the character form velocity  getvelocity 함수
        FVector Velocity{ArcherCharacter->GetVelocity()};
        Velocity.Z = 0;
        Speed = Velocity.Size();
        
        // 캐릭터가 공중에 있으면
        bIsInAir = ArcherCharacter->GetCharacterMovement()->IsFalling();
        
        // 캐릭터가 가속도가 붙었을때   Get캐릭터와 커런트엘셀, size() 는 함수이다 검색해서 공부하셈 20번째
        if (ArcherCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0)
        {
            bIsAccelerating = true;
        }
        else
        {
            bIsAccelerating = false;
        }
        
        if (ArcherCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0)
        {
            bIsSprint = true;
        }
        else
        {
            bIsSprint = false;
        }
        
        FRotator AimRotation = ArcherCharacter->GetBaseAimRotation();
        // #include "Kismet/KismetMathLibrary.h" 인클루드해라  그래야 MakeRotFromX 함수를 사용할 수 있음
        FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ArcherCharacter->GetVelocity());
        // NormalizedDeltaRotator 함수는 FRotator 리턴하고 movementOffset은 부동 소수점이다
        MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
        
        // 아처 캐릭터의 속도가 0보다 클때 실행한다
        if (ArcherCharacter->GetVelocity().Size() > 0.f)
        {
            LastMovementOffsetYaw = MovementOffsetYaw;
        }
        // 캐릭터.h 에서 get 함수를 만듬
        bAiming = ArcherCharacter->GetAiming();
        
        
        if (bReloading)
        {
            OffsetState = EOffsetState::EOS_Reloading;
        }
        else if (bIsInAir)
        {
            OffsetState = EOffsetState::EOS_InAir;
        }
        else if (ArcherCharacter->GetAiming())
        {
            OffsetState = EOffsetState::EOS_Aiming;
        }
        else
        {
            OffsetState = EOffsetState::EOS_Hip;
        }
        // 아처 캐릭터에 유효한 장착 무기가 있는지 체크한다
        if (ArcherCharacter->GetEquippedWeapon())
        {
            EquippedWeaponType = ArcherCharacter->GetEquippedWeapon()->GetWeaponType();
        }
        
        
        // 디버그를 하기위한 코드
        //FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), AimRotation.Yaw);
        //FString MovementRotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), MovementRotation.Yaw);
        //무브먼트오프셋yaw를 출력한다
        //FString OffsetMessage = FString::Printf(TEXT("Movement Offset Yaw: %f"), MovementOffsetYaw);
        /*
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, OffsetMessage);
        }
         */
    }
    TurnInPlace();
    Lean(DeltaTime);
}


void UArcherAnimInstance::NativeInitializeAnimation()
{
    // 네이티브 초기화 애니메이션에서 호출된다
    ArcherCharacter = Cast<AArcherCharacter>(TryGetPawnOwner());
}

// 캐릭터가 90도 나 -90도 때 방향을 회전한는 함수
void UArcherAnimInstance::TurnInPlace()
{
    if (ArcherCharacter == nullptr) return;
    // 상, 하 로 바라보는 캐릭터의 방향을 쓰기위한 코드
    Pitch = ArcherCharacter->GetBaseAimRotation().Pitch;
    
    
    // 점프하고나서 착지할때 하반신이 정면을 바라보지 못하고 이상한곳으로 착지한다 그럴땐 bIsInAir 변수값을 OR 하여 초기화 시켜준다
    if (Speed > 0 || bIsInAir)
    {
        // Don't want to trun in place; Character is moving
        RootYawOffset = 0.f;
        TIPCharacterYaw =ArcherCharacter->GetActorRotation().Yaw;
        TIPCharacterYawLastFrame = TIPCharacterYaw;
        RotationCurveLastFrame = 0.f;
        RotationCurve = 0.f;
    }
    else
    {
        TIPCharacterYawLastFrame = TIPCharacterYaw;
        TIPCharacterYaw = ArcherCharacter->GetActorRotation().Yaw;
        const float TIPYawDelta{TIPCharacterYaw - TIPCharacterYawLastFrame};
        
        
        // RootYawOffset  -180와 180도로 업데이트 및 고정된다
        //
        RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);
        
        // 만약 값이 1.0 일대 돌고 0.0 일땐 돌지 않는다
        const float Turning{GetCurveValue(TEXT("Turning"))};
        if (Turning > 0)
        {
            bTurningInPlace = true;
            RotationCurveLastFrame = RotationCurve;
            RotationCurve = GetCurveValue(TEXT("Rotation"));
            const float DeltaRotation{RotationCurve - RotationCurveLastFrame};
            
            // RootYawOffset > 0 의 뜻은 왼쪽으로 돌고  RootYawOffset < 0일땐 오른쪽으로 돈다
            if (RootYawOffset > 0) // 왼쪽으로 돌고
            {
                RootYawOffset -= DeltaRotation;
            }
            else // 오른쪽으로 돌고
            {
                RootYawOffset += DeltaRotation;
            }
            // FMath::Abs() 는 절대값으로 변경해준다
            const float ABSRootYawOffset{FMath::Abs(RootYawOffset)};
            if (ABSRootYawOffset > 90.f)
            {
                const float YawExcess{ABSRootYawOffset - 90.f};
                if (RootYawOffset > 0)
                {
                    RootYawOffset -= YawExcess;
                }
                else
                {
                    RootYawOffset += YawExcess;
                }
            }
            else
            {
                bTurningInPlace = false;
            }

        }
    }
    // 활의 반동을 설정
    if (bTurningInPlace)
    {
        if (bReloading || bEquipping)
        {
            RecoilWeight = 1.f;
        }
        else
        {
            RecoilWeight = 0.f;
        }
    }
    else    // 제자리에서 돌아가지 않음
    {
        if (bCrouching)
        {
            if (bReloading|| bEquipping)
            {
                RecoilWeight = 1.f;
            }
            else
            {
                RecoilWeight = 0.1f;
            }
        }
        else
        {
            if (bAiming || bReloading || bEquipping) // 화살을 재장전 하는 모션이 이상해서 bReloading 변수를 OR 하여 if문에 돌렸다
            {
                RecoilWeight = 1.f;
            }
            else
            {
                RecoilWeight = 0.5;
            }
        }
    }
}

void UArcherAnimInstance::Lean(float DeltaTime)
{
    if (ArcherCharacter == nullptr) return;
    
    CharacterRotationLastFrame = CharacterRotation;
    CharacterRotation = ArcherCharacter->GetActorRotation();
    
    const FRotator Delta{ UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame)};
    
    const float Target{Delta.Yaw / DeltaTime};
    
    const float Interp{FMath::FInterpTo(YawDelta, Target, DeltaTime, 1.5f)}; // 6
    
    YawDelta = FMath::Clamp(Interp, -90.f, 90.f);
    

}
