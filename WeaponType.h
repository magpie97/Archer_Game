#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    EWT_HolyBow UMETA(DisplayName = "HolyBow"),
    EWT_IceBow UMETA(DisplayName = "IceBow"),
    EWT_NatureBow UMETA(DisplayName = "NatureBow"),
    
    EWT_MAX UMETA(DisplayName = "DefaultMAX")
};
