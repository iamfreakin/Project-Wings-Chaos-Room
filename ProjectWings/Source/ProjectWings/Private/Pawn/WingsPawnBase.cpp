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
#include "Kismet/GameplayStatics.h"
#include "ProjectWings/ProjectWings.h"

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

	// 카메라 지연 및 회전 지연 활성화
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->bEnableCameraRotationLag = true;
	SpringArmComponent->CameraLagSpeed = 10.0f;
	SpringArmComponent->CameraRotationLagSpeed = 8.0f;

	// 카메라 설정
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	// 니아가라 트레일 설정
	EngineTrailComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EngineTrailComponent"));
	EngineTrailComponent->SetupAttachment(RootComponent);
	EngineTrailComponent->bAutoActivate = false;

	// 비행 관련 기본값 설정
	FlightPitchSensitivity = 0.05f;
	FlightYawSensitivity = 0.03f;
	FlightRollSensitivity = 0.5f;
	VelocityAlignmentSpeed = 2.0f;
	bEnableAutoLeveling = true;
	AutoLevelingSpeed = 1.5f;
	BankToTurnAmount = 0.1f;
	FlightSideMoveForce = 1500.0f;
	MaxForwardThrust = 50000.0f;
	ThrustStep = 100.0f;
	CurrentThrust = 0.0f;

	// 카메라 동적 효과 기본값
	CameraLagSpeed = 10.0f;
	CameraRotationLagSpeed = 8.0f;
	MinFOV = 90.0f;
	MaxFOV = 110.0f;
	MinArmLength = 800.0f;
	MaxArmLength = 1000.0f;
	DynamicCameraSpeedThreshold = 50000.0f;
	FreeLookSensitivity = 0.5f;
	CameraReturnSpeed = 5.0f;

	// 연료 관련 기본값 설정
	MaxFuel = 100.0f;
	CurrentFuel = 100.0f;
	FuelConsumptionRate = 1.0f;
	ThrustFuelConsumptionMultiplier = 2.0f;

	// 초기 상태 설정
	CurrentState = EWingsPawnState::Flying;
	bIsFreeLooking = false;
}

void AWingsPawnBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentFuel = MaxFuel;

	// IMC 추가 로직은 이제 AWingsPlayerController::TransitionToFlight에서 관리합니다.
}

void AWingsPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == EWingsPawnState::Flying)
	{
		// 0. 연료 소모 및 고갈 처리
		if (CurrentFuel > 0.f)
		{
			// 기본 소모 + 추진력에 따른 추가 소모
			float ThrustRatio = (MaxForwardThrust > 0.f) ? (CurrentThrust / MaxForwardThrust) : 0.f;
			float ActualConsumption = FuelConsumptionRate + (ThrustRatio * FuelConsumptionRate * ThrustFuelConsumptionMultiplier);
			
			CurrentFuel = FMath::Max(0.f, CurrentFuel - (ActualConsumption * DeltaTime));

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(2, DeltaTime, FColor::Green, 
					FString::Printf(TEXT("Fuel: %.1f%%"), GetFuelPercentage() * 100.f));
			}
		}
		else
		{
			// 연료 고갈 시 패널티
			CurrentThrust = 0.f;
			MeshComponent->SetLinearDamping(2.0f); // 공기 저항 급증 (추락 유도)
			
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(2, DeltaTime, FColor::Red, TEXT("OUT OF FUEL!"));
			}
		}

		// 1. Velocity Alignment: 진행 방향 보정
		FVector CurrentVelocity = MeshComponent->GetPhysicsLinearVelocity();
		float Speed = CurrentVelocity.Size();
		if (Speed > 100.f)
		{
			FVector TargetVelocity = GetActorForwardVector() * Speed;
			FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, DeltaTime, VelocityAlignmentSpeed);
			MeshComponent->SetPhysicsLinearVelocity(NewVelocity);
		}

		// 2. Auto-Leveling (PD 제어기)
		if (bEnableAutoLeveling)
		{
			FQuat CurrentQuat = GetActorQuat();
			FVector RightVector = CurrentQuat.GetRightVector();
			float RollError = FVector::DotProduct(RightVector, FVector::UpVector);

			FVector AngVel = MeshComponent->GetPhysicsAngularVelocityInRadians();
			float RollAngVel = FVector::DotProduct(AngVel, GetActorForwardVector());

			float P_Gain = AutoLevelingSpeed * 40.0f; 
			float D_Gain = FMath::Sqrt(P_Gain) * 2.0f;

			float LevelingTorque = (-RollError * P_Gain) - (RollAngVel * D_Gain);
			MeshComponent->AddTorqueInRadians(GetActorForwardVector() * LevelingTorque, NAME_None, true);
		}

		// 3. Bank-to-Turn
		FVector Right = GetActorRightVector();
		float RollLean = FVector::DotProduct(Right, FVector::UpVector);
		if (FMath::Abs(RollLean) > 0.05f)
		{
			float YawTorque = -RollLean * BankToTurnAmount * 10.0f;
			MeshComponent->AddTorqueInRadians(FVector::UpVector * YawTorque, NAME_None, true);
		}

		// 4. Constant Thrust
		if (CurrentThrust > 0.f)
		{
			MeshComponent->AddForce(GetActorForwardVector() * CurrentThrust, NAME_None, true);
		}

		// 5. Dynamic Camera (FOV & Distance)
		float CurrentSpeed = MeshComponent->GetPhysicsLinearVelocity().Size();
		float SpeedAlpha = FMath::Clamp(CurrentSpeed / DynamicCameraSpeedThreshold, 0.f, 1.f);

		float TargetFOV = FMath::Lerp(MinFOV, MaxFOV, SpeedAlpha);
		float TargetArmLength = FMath::Lerp(MinArmLength, MaxArmLength, SpeedAlpha);

		CameraComponent->SetFieldOfView(FMath::FInterpTo(CameraComponent->FieldOfView, TargetFOV, DeltaTime, 2.0f));
		SpringArmComponent->TargetArmLength = FMath::FInterpTo(SpringArmComponent->TargetArmLength, TargetArmLength, DeltaTime, 2.0f);

		// 6. Free Look Smooth Return
		if (!bIsFreeLooking)
		{
			FRotator CurrentRelativeRot = SpringArmComponent->GetRelativeRotation();
			if (!CurrentRelativeRot.IsNearlyZero())
			{
				FRotator NewRelativeRot = FMath::RInterpTo(CurrentRelativeRot, FRotator::ZeroRotator, DeltaTime, CameraReturnSpeed);
				SpringArmComponent->SetRelativeRotation(NewRelativeRot);
			}
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
			// 오직 비행 조종 입력만 바인딩
			EnhancedInputComponent->BindAction(InputConfig->IA_Aim, ETriggerEvent::Triggered, this, &AWingsPawnBase::Input_FlightMouse);
			EnhancedInputComponent->BindAction(InputConfig->IA_Pitch, ETriggerEvent::Triggered, this, &AWingsPawnBase::Input_PitchKeyboard);
			EnhancedInputComponent->BindAction(InputConfig->IA_Roll, ETriggerEvent::Triggered, this, &AWingsPawnBase::Input_Roll);
			EnhancedInputComponent->BindAction(InputConfig->IA_Thrust, ETriggerEvent::Triggered, this, &AWingsPawnBase::Input_Thrust);

			// 자유 시점 입력 바인딩
			EnhancedInputComponent->BindAction(InputConfig->IA_FreeLook, ETriggerEvent::Started, this, &AWingsPawnBase::Input_FreeLookStarted);
			EnhancedInputComponent->BindAction(InputConfig->IA_FreeLook, ETriggerEvent::Completed, this, &AWingsPawnBase::Input_FreeLookCompleted);
		}
	}
}

void AWingsPawnBase::SetPawnState(EWingsPawnState NewState)
{
	CurrentState = NewState;

	switch (CurrentState)
	{
	case EWingsPawnState::Flying:
		MeshComponent->SetSimulatePhysics(true);
		EngineTrailComponent->Activate();
		break;

	case EWingsPawnState::Crashed:
		// 충돌 처리 (추후 구현)
		break;
	}
}

void AWingsPawnBase::Input_FlightMouse(const FInputActionValue& Value)
{
	if (CurrentState != EWingsPawnState::Flying) return;

	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (bIsFreeLooking)
	{
		// 자유 시점 중: 스프링 암의 상대 회전값 변경
		FRotator NewRotation = SpringArmComponent->GetRelativeRotation();
		NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + (LookAxisVector.Y * FreeLookSensitivity), -80.f, 80.f);
		NewRotation.Yaw += (LookAxisVector.X * FreeLookSensitivity);
		SpringArmComponent->SetRelativeRotation(NewRotation);
	}
	else
	{
		// 일반 비행 중: 기체에 토크 부여
		float PitchTorque = LookAxisVector.Y * FlightPitchSensitivity * 25.f;
		float YawTorque = LookAxisVector.X * FlightYawSensitivity * 15.f;

		MeshComponent->AddTorqueInRadians(GetActorRightVector() * PitchTorque, NAME_None, true);
		MeshComponent->AddTorqueInRadians(GetActorUpVector() * YawTorque, NAME_None, true);
	}
}

void AWingsPawnBase::Input_PitchKeyboard(const FInputActionValue& Value)
{
	if (CurrentState != EWingsPawnState::Flying) return;

	float PitchValue = Value.Get<float>();
	float PitchTorque = PitchValue * FlightPitchSensitivity * 30.f;
	MeshComponent->AddTorqueInRadians(GetActorRightVector() * PitchTorque, NAME_None, true);
}

void AWingsPawnBase::Input_Roll(const FInputActionValue& Value)
{
	if (CurrentState != EWingsPawnState::Flying) return;

	float RollValue = Value.Get<float>();
	float RollTorque = RollValue * FlightRollSensitivity * 30.f;
	MeshComponent->AddTorqueInRadians(GetActorForwardVector() * RollTorque, NAME_None, true);

	FVector SideMoveForce = GetActorRightVector() * RollValue * FlightSideMoveForce * 0.01f;
	MeshComponent->AddForce(SideMoveForce, NAME_None, true);
}

void AWingsPawnBase::Input_Thrust(const FInputActionValue& Value)
{
	if (CurrentState != EWingsPawnState::Flying || CurrentFuel <= 0.f) return;

	float ThrustInput = Value.Get<float>();
	CurrentThrust = FMath::Clamp(CurrentThrust + (ThrustInput * ThrustStep), 0.f, MaxForwardThrust);
}

void AWingsPawnBase::Input_FreeLookStarted(const FInputActionValue& Value)
{
	bIsFreeLooking = true;
}

void AWingsPawnBase::Input_FreeLookCompleted(const FInputActionValue& Value)
{
	bIsFreeLooking = false;
}
