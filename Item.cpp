// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "ArcherCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


// Sets default values
AItem::AItem():
ItemName(FString("활")),
ItemCount(0),
ItemState(EItemState::EIS_Pickup),
ItemInterpStartLocation(FVector(0.f)),
CameraTargetLocation(FVector(0.f)),
bInterping(false),
ZCurveTime(0.7f),
ItemInterpX(0.f),
ItemInterpY(3.f),
InterpInitialYawOffset(0.f),
SlotIndex(0),
bCharacterInventoryFull(false)

//ItemAbility(EItemAbility::EIA_Holy) // 나중에 쓰자
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    // 아이템 매쉬 만들기
    ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
    SetRootComponent(ItemMesh);
    
    // 아이템의 콜리전 박스 만들기 // BoxComponent.h를 인클루드한다
    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    CollisionBox->SetupAttachment(ItemMesh);
    CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

    // 위젯 만들고 붙이기
    PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
    PickupWidget->SetupAttachment(GetRootComponent());
    
    AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
    AreaSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
    
    // 위젯을 숨기기
    if (PickupWidget)
    {
        PickupWidget->SetVisibility(false);
    }
    /*
    // ActiveAbilitys 배열 아이템을 설정 나중에 쓰자
    SetActiveAbilitys();
    */
    // OnComponentbeginoverlap은 F유형 componentbeginOverlap 시그니처가 있는 구조체이다
    AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
    AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
    
    // ItemState를 기반으로 Item 프로퍼티들들 설정한다
    SetItemProperties(ItemState);
	
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverLappedComponent,
                     AActor* OtherActor,
                     UPrimitiveComponent* OtherComp,
                     int32 OtherBodyIndex,
                     bool bFromSweep,
                     const FHitResult& SweepResult)
{
    if (OtherActor)
    {
        AArcherCharacter* ArcherCharacter = Cast<AArcherCharacter>(OtherActor);
        if (ArcherCharacter)
        {
            ArcherCharacter->IncrementOverlappedItemCount(1);
        }
    }
}


void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverLappedComponent,
                        AActor* OtherActor,
                        UPrimitiveComponent* OtherComp,
                        int32 OtherBodyIndex)
{
    if (OtherActor)
    {
        AArcherCharacter* ArcherCharacter = Cast<AArcherCharacter>(OtherActor);
        if (ArcherCharacter)
        {
            ArcherCharacter->IncrementOverlappedItemCount(-1);
            ArcherCharacter->UnHighlightInventorySlot();
        }
    }
}

void AItem::SetItemProperties(EItemState State)
{
    switch (State)
    {
            // 블루프린트 버그가 있음 75 하지만 에디터를 다시 시작하면 해결되는거 같음
        case EItemState::EIS_Pickup:
            // 매쉬 프로퍼티를 설정한다
            ItemMesh->SetSimulatePhysics(false);
            ItemMesh->SetEnableGravity(false);
            ItemMesh->SetVisibility(true);
            ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            // AreaSphere properties를 설정한다
            AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
            AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            // 콜리전박스 프로퍼티를 설정한다
            CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
            CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
        case EItemState::EIS_Equipped:
            PickupWidget->SetVisibility(false);
            // 매쉬 프로퍼티를 설정한다
            ItemMesh->SetSimulatePhysics(false);
            ItemMesh->SetEnableGravity(false);
            ItemMesh->SetVisibility(true);
            ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            // AreaSphere properties를 설정한다
            AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            // 콜리전박스 프로퍼티를 설정한다
            CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
        case EItemState::EIS_Falling:
            // 매쉬 프로퍼티들을 설정한다
            ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            ItemMesh->SetSimulatePhysics(true);
            ItemMesh->SetEnableGravity(true);
            ItemMesh->SetVisibility(true);
            ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
            // AreaSphere properties를 설정한다
            AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            // 콜리전박스 프로퍼티를 설정한다
            CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
        case EItemState::EIS_EquipInterping:
            PickupWidget->SetVisibility(false);
            // 매쉬 프로퍼티를 설정한다
            ItemMesh->SetSimulatePhysics(false);
            ItemMesh->SetEnableGravity(false);
            ItemMesh->SetVisibility(true);
            ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            // AreaSphere properties를 설정한다
            AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            // 콜리전박스 프로퍼티를 설정한다
            CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
        case EItemState::EIS_PickedUp:
            PickupWidget->SetVisibility(false);
            // 매쉬 프로퍼티를 설정한다
            ItemMesh->SetSimulatePhysics(false);
            ItemMesh->SetEnableGravity(false);
            ItemMesh->SetVisibility(false);
            ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            // AreaSphere properties를 설정한다
            AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            // 콜리전박스 프로퍼티를 설정한다
            CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            
            break;
            
    }
}

/* 잠시 남겨놓고 쓸때 있으면 쓰자
void AItem::SetActiveAbilitys()
{
    for (int32 i = 0; i <= 4; i++)
    {
        ActiveAbilitys.Add(false);
    }
    
    switch (ItemAbility)
    {
        case EItemAbility::EIA_Damaged:
            ActiveAbilitys[1] = true;
            break;
        case EItemAbility::EIA_Holy:
            ActiveAbilitys[1] = true;
            ActiveAbilitys[2] = true;
            break;
        case EItemAbility::EIA_Ice:
            ActiveAbilitys[1] = true;
            ActiveAbilitys[2] = true;
            ActiveAbilitys[3] = true;
            break;
        case EItemAbility::EIA_Nature:
            ActiveAbilitys[1] = true;
            ActiveAbilitys[2] = true;
            ActiveAbilitys[3] = true;
            ActiveAbilitys[4] = true;
    }
}
*/


void AItem::FinishInterping()
{
    bInterping = false;
    if (Character)
    {
        Character->GetPickupItem(this);
        
        Character->UnHighlightInventorySlot();
    }
    // 스케일을 다시 정상으로 설정
    SetActorScale3D(FVector(1.f));
}

void AItem::ItemInterp(float DeltaTime)
{
    if (!bInterping) return;
    
    if (Character && ItemZCurve)
    {
        // ItemInterpTimer를 시작한 이후 경과된 시간
        const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
        // 경과 시간에 해당하는 Curve 값을 가져온다
        const float CurveValue = ItemZCurve->GetFloatValue(ElapsedTime);
        // Cruve가 시작될때 item의 초기 위치를 가져온다
        FVector ItemLocation = ItemInterpStartLocation;
        // 카메라 앞의 위치를 가져오기
        const FVector CameraInterpLocation{Character->GetCameraInterpLocation()};
        // item에서 카메라 interp 위치까지의 벡터, X 밑 Y는 0값으로 설정한다
        const FVector ItemToCamera{FVector(0.f, 0.f, (CameraInterpLocation - ItemLocation).Z)};
        // Curve 값과 곱할 배율 인수
        const float DeltaZ = ItemToCamera.Size();
        
        const FVector CurrentLocation{GetActorLocation()};
        const float InterpXValue = FMath::FInterpTo(CurrentLocation.X, CameraInterpLocation.X, DeltaTime, 30.0f); // 30은 보간 속도
        const float InterpYValue = FMath::FInterpTo(CurrentLocation.Y, CameraInterpLocation.Y, DeltaTime, 30.0f);
        
        // Set X and Y of ItemLocation to Interped Value
        ItemLocation.X = InterpXValue;
        ItemLocation.Y = InterpYValue;
        
        // 초기 위치의 Z Component에 Curve 값을 추가 (DeltaZ에 의해 조정됨)
        ItemLocation.Z +=CurveValue * DeltaZ;
        SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);
        
        // 프레임의 카메라 회전
        const FRotator CameraRotation{Character->GetFollowCamera()->GetComponentRotation()};
        // 카메라 회전과 초기 Yaw 오프셋
        FRotator ItemRotation{ 0.f, CameraRotation.Yaw + InterpInitialYawOffset, 0.f};
        SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);
        
        if (ItemScaleCurve)
        {
            const float ScaleCurveValue = ItemScaleCurve->GetFloatValue(ElapsedTime);
            SetActorScale3D(FVector(ScaleCurveValue, ScaleCurveValue, ScaleCurveValue));
        }
        
    }
}

void AItem::PlayPickupSound(bool bForcePlaySound)
{
    if (Character)
    {
        if (bForcePlaySound)
        {
            if (PickupSound)
            {
                UGameplayStatics::PlaySound2D(this, PickupSound);
            }
        }
        else if (Character->ShouldPlayPickupSound())
        {
            Character->StartPickupSoundTimer();
            if (PickupSound)
            {
                UGameplayStatics::PlaySound2D(this, PickupSound);
            }
        }
    }
}

void AItem::PlayEquipSound(bool bForcePlaySound)
{
    if (Character)
    {
        if (bForcePlaySound)
        {
            if (EquipSound)
            {
                UGameplayStatics::PlaySound2D(this, EquipSound);
            }
        }
        else if (Character->ShouldPlayEquipSound())
        {
            Character->StartEquipSoundTimer();
            if (EquipSound)
            {
                UGameplayStatics::PlaySound2D(this, EquipSound);
            }
        }
    }
}


// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
    // EquipInterping 상태일때 아이템 인터핑 처리를 한다
    ItemInterp(DeltaTime);

}

void AItem::SetItemState(EItemState State)
{
    ItemState = State;
    SetItemProperties(State);
}

void AItem::StartItemCurve(AArcherCharacter* Char, bool bForcePlaySound)
{
    // 캐릭터에 대한 handle을 저장
    Character = Char;
    
    PlayPickupSound(bForcePlaySound);
    
    // store initial location of the item
    ItemInterpStartLocation = GetActorLocation();
    
    bInterping = true;
    SetItemState(EItemState::EIS_EquipInterping);
    
    GetWorldTimerManager().SetTimer(ItemInterpTimer, this, &AItem::FinishInterping, ZCurveTime);
    
    // 카메라의 초기 Yaw 값 가져오기
    const float CameraRotationYaw{Character->GetFollowCamera()->GetComponentRotation().Yaw};
    // 아이템의 초기 Yaw 값 가져오기
    const float ItemRotationYaw{GetActorRotation().Yaw};
    // 카메라와 아이템 사이의 초기 Yaw 오프셋
    InterpInitialYawOffset = ItemRotationYaw - CameraRotationYaw;
}

