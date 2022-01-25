// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WeaponType.h"
#include "ArcherAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8
{
    EOS_Aiming UMETA(DisplayName = "Aiming"),
    EOS_Hip UMETA(DisplayName = "Hip"),
    EOS_Reloading UMETA(DisplayName = "Reloading"),
    EOS_InAir UMETA(DisplayName = "InAir"),
    
    EOS_MAX UMETA(DisplayName = "DefaultMAX")
};
/**
 *
 */
UCLASS()
class ARCHER_API UArcherAnimInstance : public UAnimInstance
{
    GENERATED_BODY()
    
public:
    UArcherAnimInstance();
    
    //애니메이션 속성 업데이트
    UFUNCTION(BlueprintCallable)
    void UpdateAnimationProperties(float DeltaTime);
    
    // 네이티브 초기화 애니메이션
    virtual void NativeInitializeAnimation() override;
    
protected:
    // 캐릭터의 위치가 Turn 할때 변수값들
    void TurnInPlace();
    
    // 캐릭터가 직진중 마우스를 좌, 우로 움직일때 양 옆으로 기울어 걷거나 뛰게 하는 함수
    void Lean(float DeltaTime);
    
private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    class AArcherCharacter* ArcherCharacter;
    
    // 캐릭터 속도
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    float Speed;
    
    // 공중에 있을때
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;
    
    // 캐릭터의 이동 할때
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    bool bIsAccelerating;
    
    // 활을 쏠때쓰는 오프셋 Yaw
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, Meta = (AllowPrivateAccess = "true"))
    float MovementOffsetYaw;
    
    // 아처 캐릭터가 좌 우로 움직이고 멈출때
    // 이동을 멈추기 전 프레임 오프셋 Yaw
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, Meta = (AllowPrivateAccess = "true"))
    float LastMovementOffsetYaw;
    
    // 조준을 했을때와 안했을때의 모션
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, Meta = (AllowPrivateAccess = "true"))
    bool bAiming;

    
    // 캐릭터의 Yaw 프레임   TIP의 줄인말은 return in place의 준말이다
    float TIPCharacterYaw;
    
    // 캐릭터의 Yaw 이전프레임   TIP의 줄인말은 return in place의 준말이다
    float TIPCharacterYawLastFrame;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ture In Place", Meta = (AllowPrivateAccess = "true"))
    float RootYawOffset;
    
    // 회전 커브값
    float RotationCurve;
    
    // 회전 커브 마지막 값
    float RotationCurveLastFrame;
    
    //조준중에 캐릭터의 상하를 조준하기 위한 변수 //
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ture In Place", Meta = (AllowPrivateAccess = "true"))
    float Pitch;
    
    // 활의 화살을 재장전할때 ture값 , 만약 하늘이나 땅을 보고 재장전을 한다면 평상시 Idle 포즈로 돌아가서 재장전 시퀀스를 실행하는 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ture In Place", Meta = (AllowPrivateAccess = "true"))
    bool bReloading;
    
    // Offset state; used to determine which Aim offset to use
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ture In Place", Meta = (AllowPrivateAccess = "true"))
    EOffsetState OffsetState;
    
    // Character Yaw this frame
    FRotator CharacterRotation;
    
    // Character Yaw Last frame
    FRotator CharacterRotationLastFrame;
    
    // 캐릭터의 런닝 블렌드 스페이스에 양옆으로 기우는데에 사용되는 YawDelta 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Lean, Meta = (AllowPrivateAccess = "true"))
    float YawDelta;
    
    // 웅크리기를 실행하기 위한 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Crouching, Meta = (AllowPrivateAccess = "true"))
    bool bCrouching;
    
    // 장비 실행하기 위한 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Crouching, Meta = (AllowPrivateAccess = "true"))
    bool bEquipping;
    
    // 재자리에서 기본 자세에서 화살을 발사하는 기본 반동을 바꾼다
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, Meta = (AllowPrivateAccess = "true"))
    float RecoilWeight;
    
    // 자리에서 돌때 True 값
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, Meta = (AllowPrivateAccess = "true"))
    bool bTurningInPlace;
    
    // 현재 장착된 무기의 무기 종류
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, Meta = (AllowPrivateAccess = "true"))
    EWeaponType EquippedWeaponType;
    
    // 화살을 재장전 또는 장착하지 않을 때 true
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, Meta = (AllowPrivateAccess = "true"))
    bool bShouldUseFABRIK;
    
    // 캐릭터가 뛰고 있을때
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    bool bIsSprint;
    
    
    
    
};
