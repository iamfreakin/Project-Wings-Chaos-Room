// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/WingsPawnBase.h"
#include "ProjectWings.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "NiagaraComponent.h"
#include "Core/WingsGameState.h"

AWingsPawnBase::AWingsPawnBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 루트 컴포넌트: 스태틱 메쉬
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetNotifyRigidBodyCollision(true); // 충돌 이벤트 활성화

	// 스프링 암 설정
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 600.f;
	SpringArmComponent->bUsePawnControlRotation = false; // Pawn 회전과 별도로 동작 가능하게 설정
	SpringArmComponent->bInheritPitch = true;
	SpringArmComponent->bInheritYaw = true;
	SpringArmComponent->bInheritRoll = true;

	// 카메라 설정
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	// 니아가라 트레일 설정
	EngineTrailComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EngineTrailComponent"));
	EngineTrailComponent->SetupAttachment(RootComponent);
	EngineTrailComponent->bAutoActivate = false;
}

void AWingsPawnBase::BeginPlay()
{
	Super::BeginPlay();
	
	// GameState로부터 풍향 정보 확인 테스트
	if (AWingsGameState* GS = GetWorld()->GetGameState<AWingsGameState>())
	{
		UE_LOG(LogWings, Display, TEXT("AWingsPawnBase: Global Wind Vector is %s"), *GS->GlobalWindVector.ToString());
	}
}

void AWingsPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWingsPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
