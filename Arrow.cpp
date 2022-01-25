// Fill out your copyright notice in the Description page of Project Settings.


#include "Arrow.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "ArcherCharacter.h"

AArrow::AArrow()
{
    // ArrowMesh 컴포넌트를 구성하고 Root로 설정한다
    ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
    SetRootComponent(ArrowMesh);
    
    // Item.h에서 만든 Get 함수를 사용함
    GetCollisionBox()->SetupAttachment(GetRootComponent());
    GetPickupWidget()->SetupAttachment(GetRootComponent());
    GetAreaSphere()->SetupAttachment(GetRootComponent());
    
    ArrowCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ArrowCollisionSphere"));
    ArrowCollisionSphere->SetupAttachment(GetRootComponent());
    ArrowCollisionSphere->SetSphereRadius(50.f);
}



void AArrow::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AArrow::BeginPlay()
{
    Super::BeginPlay();
    
    ArrowCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AArrow::ArrowSphereOverlap);
    
}

void AArrow::SetItemProperties(EItemState State)
{
    Super::SetItemProperties(State);
    
    switch (State)
    {
            // 블루프린트 버그가 있음 75 하지만 에디터를 다시 시작하면 해결되는거 같음
        case EItemState::EIS_Pickup:
            // 매쉬 프로퍼티를 설정한다
            ArrowMesh->SetSimulatePhysics(false);
            ArrowMesh->SetEnableGravity(false);
            ArrowMesh->SetVisibility(true);
            ArrowMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
        case EItemState::EIS_Equipped:
            // 매쉬 프로퍼티를 설정한다
            ArrowMesh->SetSimulatePhysics(false);
            ArrowMesh->SetEnableGravity(false);
            ArrowMesh->SetVisibility(true);
            ArrowMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
        case EItemState::EIS_Falling:
            // 매쉬 프로퍼티들을 설정한다
            ArrowMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            ArrowMesh->SetSimulatePhysics(true);
            ArrowMesh->SetEnableGravity(true);
            ArrowMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            ArrowMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
            break;
        case EItemState::EIS_EquipInterping:
            // 매쉬 프로퍼티를 설정한다
            ArrowMesh->SetSimulatePhysics(false);
            ArrowMesh->SetEnableGravity(false);
            ArrowMesh->SetVisibility(true);
            ArrowMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
    }
}

void AArrow::ArrowSphereOverlap(UPrimitiveComponent* OverLappedComponent,
                     AActor* OtherActor,
                     UPrimitiveComponent* OtherComp,
                     int32 OtherBodyIndex,
                     bool bFromSweep,
                     const FHitResult& SweepResult)
{
    if (OtherActor)
    {
        auto OverlappedCharacter = Cast<AArcherCharacter>(OtherActor);
        if (OverlappedCharacter)
        {
            StartItemCurve(OverlappedCharacter);
            ArrowCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
}
