// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon() :
ThrowWeaponTime(0.7f),
bFalling(false),
Arrow(1),
ArrowCaseCapacity(1),// 화살을 최대 가지고 있을 수 있는 화살의 양
WeaponType(EWeaponType::EWT_HolyBow), // 무기 종류 타입
ArrowType(EArrowType::EAT_HolyArrow),    // 화살 종류 타입
ReloadMontageSection(FName(TEXT("Reload Bow")))
{
    PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 활 무기를 던지면 무기의 피직스에 의해 월드에 세워진다
    if (GetItemState() == EItemState::EIS_Falling && bFalling)
    {
        const FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
        GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
    }
}

void AWeapon::ThrowWeapon()
{
    FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
    GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

    const FVector MeshForward{ GetItemMesh()->GetForwardVector() };
    const FVector MeshRight{ GetItemMesh()->GetRightVector() };
    // 활 무기를 -20 방향으로 던진다
    FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);
    // 활 무기를 던질때 각도 // 230도
    float RandomRotation{ 230.f };
    ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
    ImpulseDirection *= 20'000.f;
    GetItemMesh()->AddImpulse(ImpulseDirection);

    bFalling = true;
    GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);

}

void AWeapon::StopFalling()
{
    bFalling = false;
    SetItemState(EItemState::EIS_Pickup);
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    const FString WeaponTablePath{ TEXT("DataTable'/Game/_Game/DataTable/WeaponDataTable.WeaponDataTable'") };
    UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));

    if (WeaponTableObject)
    {
        FWeaponDataTable* WeaponDataRow = nullptr;
        switch (WeaponType)
        {
        case EWeaponType::EWT_HolyBow:
            WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("HolyBow"), TEXT(""));
            break;
        case EWeaponType::EWT_IceBow:
            WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("IceBow"), TEXT(""));
            break;
        case EWeaponType::EWT_NatureBow:
            WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("NatureBow"), TEXT(""));
            break;
        }

        if (WeaponDataRow)
        {
            ArrowType = WeaponDataRow->ArrowType;
            Arrow = WeaponDataRow->WeaponArrow;
            ArrowCaseCapacity = WeaponDataRow->ArrowCaseCapacity;
            SetPickupSound(WeaponDataRow->PickupSound);
            SetEquipSound(WeaponDataRow->EquipSound);
            GetItemMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
            SetItemName(WeaponDataRow->ItemName);
            SetIconItem(WeaponDataRow->InventoryIcon);
            SetArrowIcon(WeaponDataRow->ArrowIcon);
            
            CrosshairsMiddle = WeaponDataRow->CrosshairsMiddle;
            CrosshairsLeft = WeaponDataRow->CrosshairsLeft;
            CrosshairsRight = WeaponDataRow->CrosshairsRight;
            CrosshairsBottom = WeaponDataRow->CrosshairsBottom;
            CrosshairsTop = WeaponDataRow->CrosshairsTop;
            AutoFireRate = WeaponDataRow->AutoFireRate;
            BowEffect = WeaponDataRow->BowEffect;
            FireSound = WeaponDataRow->FireSound;
            Damage = WeaponDataRow->Damage;
            HeadShotDamage = WeaponDataRow->HeadShotDamage;
        }
    }
}


void AWeapon::DecrementArrow()
{
    if (Arrow - 1 <= 0)
    {
        Arrow = 0;
    }
    else
    {
        --Arrow;
    }
}

void AWeapon::ReloadArrow(int32 Amount)
{
    checkf(Arrow + Amount <= ArrowCaseCapacity, TEXT("Attempted to reload with more then ArrowCase Capacity!"));
    Arrow += Amount;
    
}

bool AWeapon::ArrowCaseIsFull()
{
    return Arrow >= ArrowCaseCapacity;
}
