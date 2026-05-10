// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/WingsPawnBase.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "NiagaraComponent.h"
#include "Core/WingsGameState.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Data/WingsInputConfigData.h"

// 로그 사용을 위해 필요한 경우에만 선언 (ProjectWings.h 대체)
DECLARE_LOG_CATEGORY_EXTERN(LogWings, Log, All);

AWingsPawnBase::AWingsPawnBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 루트 컴포넌트: 스태틱 메쉬
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetNotifyRigidBodyCollision(true);

	MeshComponent->SetLinearDamping(0.5f);
	MeshComponent->SetAngularDamping(1.0f);

	// 스프링 암 설정
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 800.f;
	SpringArmComponent->SocketOffset = FVector(0.f, 0.f, 100.f);
	SpringArmComponent->bUsePawnControlRotation = false;
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

	// 기본 스탯 초기화
	InitialLaunchForce = 500000.f;
	MaxLaunchForce = 1500000.f; // 최대 150만
	ChargeSpeed = 0.5f;         // 2초면 풀충전
	CurrentLaunchPower = 0.f;
	bIsCharging = false;
	AimRotationSpeed = 2.f;

	// 초기 상태 설정
	CurrentState = EWingsPawnState::Ready;
}

void AWingsPawnBase::BeginPlay()
{
	Super::BeginPlay();

	// 상태 초기화 로직 실행
	SetPawnState(EWingsPawnState::Ready);

	// Enhanced Input Subsystem에 매핑 컨텍스트 추가
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	if (AWingsGameState* GS = GetWorld()->GetGameState<AWingsGameState>())
	{
		UE_LOG(LogWings, Display, TEXT("AWingsPawnBase: Global Wind Vector is %s"), *GS->GlobalWindVector.ToString());
	}
}

void AWingsPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 파워 충전 로직
	if (bIsCharging && CurrentState == EWingsPawnState::Ready)
	{
		CurrentLaunchPower = FMath::Clamp(CurrentLaunchPower + (ChargeSpeed * DeltaTime), 0.f, 1.f);
		
		// 화면에 충전 게이지 표시 (디버그용)
		if (GEngine)
		{
			FString PowerMsg = FString::Printf(TEXT("Charging Power: %.2f"), CurrentLaunchPower);
			GEngine->AddOnScreenDebugMessage(1, DeltaTime, FColor::Yellow, PowerMsg);
		}
	}
}

void AWingsPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (InputConfig)
		{
			// Aim (Axis2D)
			EnhancedInputComponent->BindAction(InputConfig->IA_Aim, ETriggerEvent::Triggered, this, &AWingsPawnBase::Input_Aim);

			// Launch (Started: 충전 시작 / Completed: 발사)
			EnhancedInputComponent->BindAction(InputConfig->IA_Launch, ETriggerEvent::Started, this, &AWingsPawnBase::Input_LaunchStarted);
			EnhancedInputComponent->BindAction(InputConfig->IA_Launch, ETriggerEvent::Completed, this, &AWingsPawnBase::Input_LaunchCompleted);
		}
	}
}

void AWingsPawnBase::SetPawnState(EWingsPawnState NewState)
{
	CurrentState = NewState;

	switch (CurrentState)
	{
	case EWingsPawnState::Ready:
		// 조준 중에는 중력 영향 없이 허공에 고정
		MeshComponent->SetSimulatePhysics(false);
		CurrentLaunchPower = 0.f;
		bIsCharging = false;
		break;

	case EWingsPawnState::Flying:
		// 발사 시 물리 시뮬레이션 활성화
		MeshComponent->SetSimulatePhysics(true);
		EngineTrailComponent->Activate();
		break;

	case EWingsPawnState::Crashed:
		// 충돌 후 제어 불능 (추후 구현)
		break;
	}
}

void AWingsPawnBase::Input_Aim(const FInputActionValue& Value)
{
	if (CurrentState != EWingsPawnState::Ready) return;

	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (LookAxisVector.IsNearlyZero()) return;

	// 마우스 이동값에 따른 회전 계산 (Pitch, Yaw)
	FRotator CurrentRotation = GetActorRotation();
	float NewPitch = FMath::Clamp(CurrentRotation.Pitch + (LookAxisVector.Y * AimRotationSpeed), -60.f, 60.f);
	float NewYaw = CurrentRotation.Yaw + (LookAxisVector.X * AimRotationSpeed);

	SetActorRotation(FRotator(NewPitch, NewYaw, 0.f));
}

void AWingsPawnBase::Input_LaunchStarted(const FInputActionValue& Value)
{
	if (CurrentState == EWingsPawnState::Ready)
	{
		bIsCharging = true;
		CurrentLaunchPower = 0.f;
	}
}

void AWingsPawnBase::Input_LaunchCompleted(const FInputActionValue& Value)
{
	if (bIsCharging && CurrentState == EWingsPawnState::Ready)
	{
		bIsCharging = false;
		SetPawnState(EWingsPawnState::Flying);

		// 충전된 파워에 따른 최종 힘 계산 (최소 InitialLaunchForce 보장)
		float FinalForce = FMath::Max(MaxLaunchForce * CurrentLaunchPower, InitialLaunchForce);
		
		FVector LaunchDirection = GetActorForwardVector();
		MeshComponent->AddImpulse(LaunchDirection * FinalForce, NAME_None, true);

		UE_LOG(LogWings, Display, TEXT("AWingsPawnBase: Launched with power %.2f (Total Force: %f)"), CurrentLaunchPower, FinalForce);
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(2, 2.f, FColor::Cyan, FString::Printf(TEXT("Launched! Power: %.2f"), CurrentLaunchPower));
		}
	}
}

