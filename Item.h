// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemState : uint8
{
    EIS_Pickup UMETA(DisplayName = "Pickup"),
    EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
    EIS_PickedUp UMETA(DisplayName = "PickedUp"),
    EIS_Equipped UMETA(DisplayName = "Equipped"),
    EIS_Falling UMETA(DisplayName = "Falling"),

    
    EIS_MAX UMETA(DisplayName = "DefaultMAX")
};



/* 나중에 쓰자
UENUM(BlueprintType)
enum class EItemAbility : uint8
{
    // 'EIA_'는 EItemAbility의 줄여서 작성한것
    EIA_Damaged UMETA(DisplayName = "Damaged"),
    EIA_Holy UMETA(DisplayName = "Holy"),
    EIA_Ice UMETA(DisplayName = "Ice"),
    EIA_Nature UMETA(DisplayName = "Nature"),
    
    EIA_MAX UMETA(DisplayName = "MAX")
};
 */

UCLASS()
class ARCHER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    
    // AreaSphere를 겹칠 때 호출됨
    UFUNCTION()
    void OnSphereOverlap(UPrimitiveComponent* OverLappedComponent,
                         AActor* OtherActor,
                         UPrimitiveComponent* OtherComp,
                         int32 OtherBodyIndex,
                         bool bFromSweep,
                         const FHitResult& SweepResult);
    // AreaSphere를 종료 할때 호출됨
    UFUNCTION()
    void OnSphereEndOverlap(UPrimitiveComponent* OverLappedComponent,
                            AActor* OtherActor,
                            UPrimitiveComponent* OtherComp,
                            int32 OtherBodyIndex);
    
    // 상태에 따라 ItemComponents을 설정한다
    virtual void SetItemProperties(EItemState State);
    
    
    void FinishInterping();
    
    // EquipInterping 상태일때 아이템을 보간처리 한다
    void ItemInterp(float DeltaTime);
    
    // 능력 이미지를 기반으로 bool의 ActiveAbilitys 배열을 설정한다  나중에 쓰자
    //void SetActiveAbilitys();
    
    void PlayPickupSound(bool bForcePlaySound = false);


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
    
    // AArcherCharacter::GetPickupItem을 불러온다
    void PlayEquipSound(bool bForcePlaySound = false);
    
private:
    // Item을 위한 skeletal 매쉬
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    USkeletalMeshComponent* ItemMesh;
    
    // 라인 트레이스가 콜리전 박스에 충돌하면 아이템의 정보를 위젯으로 표시하게 만든다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    class UBoxComponent* CollisionBox;

	// 플레이어가 무기를 봤을대 무기 정보를 표시할 위젯을 나타낸다  그리고 위젯을 사용하고 자 할때 파일이름.Build.cs 파일로 가서 ,"UMG" 를 추가한다
	// UMG는 Unreal Motion Graphics 의 줄인말이다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
	class UWidgetComponent* PickupWidget;
    
    // 겹칠 때 item 추적을 활성화 한다
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    class USphereComponent* AreaSphere;
    
    // 위젯에 띄울 아이템 이름
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    FString ItemName;
    
    // 화살 아이템 겟수 표시
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    int32 ItemCount;
    
    // 아이템의 상태
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    EItemState ItemState;
    
    
    
    
    //EditDefaultsOnly는  블루프린트에서만 속성을 부여할 수 있다
    // interping 할때 아이템의 Z 위치에서 사용할 Curve 에셋이다
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    class UCurveFloat* ItemZCurve;
    
    // e키를 눌러 아이템을 먹을때 보간 커브를 시작하는 위치를 정할때 사용하는 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    FVector ItemInterpStartLocation;
    // e키를 눌러 아이템을 먹을때 커브를 시작하고 도착하는 카메라 위치로 이동하는 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    FVector CameraTargetLocation;
    
    // 인터핑이 시작할때 FTimerHandle이 시작 된다
    FTimerHandle ItemInterpTimer;
    // 보간 커브가 작동 중일대  true
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    bool bInterping;
    
    // curve 및 timer의 지속 시간을 정할때 쓰는 변수
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    float ZCurveTime;
    
    // 포인터로 캐릭터를 가르킨다
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    class AArcherCharacter* Character;
    
    float ItemInterpX;
    float ItemInterpY;
    
    // 카메라와 interping item 사이의 초기 Yaw 오프셋
    float InterpInitialYawOffset;
    
    // 보간 중일때 아이템의 크기를 조정하는데 사용되는 Curve 변수
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    UCurveFloat* ItemScaleCurve;
    
    // 활 무기를 집었을때 소리나게 한다
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    class USoundCue* PickupSound;
    
    // 활이 장착하고 바뀔때 나는 소리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    USoundCue* EquipSound;
    
    // 인벤토리의 아이템의 배경을 변경
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "ture"))
    UTexture2D* IconBackground;
    
    // 인벤토리의 아이템 아이콘은 변경
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "ture"))
    UTexture2D* IconItem;
    
    // 인벤토리의 아이템의 화살아이콘을 변경
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "ture"))
    UTexture2D* ArrowItem;
    
    // 인벤토리 array의 슬롯
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "ture"))
    int32 SlotIndex;
    
    // 캐릭터의 인벤토리가 꽉차면 true
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "ture"))
    bool bCharacterInventoryFull;
    

    
    /*
    // 나중에 쓰자 68번
    // 위젯에 활의 능력 이미지를 표시한다 holy, ice, nature 등등 추가할 수 있으면 더 추가
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    EItemAbility ItemAbility;
    
    // 나중에 쓰자
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "ture"))
    TArray<bool> ActiveAbilitys;
    */
    
    
    
    
public:// 겟터 함수
    FORCEINLINE UWidgetComponent* GetPickupWidget() const {return PickupWidget; }
    FORCEINLINE USphereComponent* GetAreaSphere() const {return AreaSphere;}
    FORCEINLINE UBoxComponent* GetCollisionBox() const {return CollisionBox;}
    FORCEINLINE EItemState GetItemState() const {return ItemState;}
    void SetItemState(EItemState State);
    FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh;}
    FORCEINLINE USoundCue* GetPickupSound() const {return PickupSound;}
    FORCEINLINE void SetPickupSound(USoundCue* Sound) {PickupSound = Sound;}
    FORCEINLINE USoundCue* GetEquipSound() const {return EquipSound;}
    FORCEINLINE void SetEquipSound(USoundCue* Sound) {EquipSound = Sound;}
    FORCEINLINE int32 GetItemCount() const {return ItemCount;}
    FORCEINLINE int32 GetSlotIndex() const {return SlotIndex;}
    FORCEINLINE void SetSlotIndex(int32 Index) {SlotIndex = Index;}
    FORCEINLINE void SetCharacter(AArcherCharacter* Char) {Character = Char;}
    FORCEINLINE void SetCharacterInventoryFull(bool bFull) {bCharacterInventoryFull = bFull;}
    FORCEINLINE void SetItemName(FString Name) {ItemName = Name;}
    // 인벤토리의 아이템 아이콘 설정
    FORCEINLINE void SetIconItem(UTexture2D* Icon) {IconItem = Icon;}
    // 위젯의 화살 아이콘 설정
    FORCEINLINE void SetArrowIcon(UTexture2D* Icon) {ArrowItem = Icon;}
    
    // AArcherCharacter 클래스를 불러온다
    void StartItemCurve(AArcherCharacter* Char, bool bForcePlaySound = false);  // 

};
