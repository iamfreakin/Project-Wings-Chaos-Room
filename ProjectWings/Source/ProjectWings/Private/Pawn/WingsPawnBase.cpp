// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/WingsPawnBase.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Core/WingsGameState.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Data/WingsInputConfigData.h"
#include "Data/WingsFlightData.h"
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

	// 기본 상태 설정
	bEnableAutoLeveling = true;
	CurrentThrust = 0.0f;

	// 카메라 동적 효과 기본값 (Data Asset 미설정 시 대비)
	CameraLagSpeed = 10.0f;
	CameraRotationLagSpeed = 8.0f;
	MinFOV = 90.0f;
	MaxFOV = 110.0f;
	MinArmLength = 800.0f;
	MaxArmLength = 1000.0f;
	DynamicCameraSpeedThreshold = 50000.0f;
	FreeLookSensitivity = 0.5f;

	// 연료 관련 기본값 설정
	MaxFuel = 100.0f;
	CurrentFuel = 100.0f;

	// 초기 상태 설정
	CurrentState = EWingsPawnState::Flying;
	bIsFreeLooking = false;
}

void AWingsPawnBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentFuel = MaxFuel;

	if (EngineTrailComponent)
	{
		EngineTrailComponent->Activate();
	}
}

void AWingsPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == EWingsPawnState::Flying)
	{
		// 0. 연료 소모 및 고갈 처리
		if (CurrentFuel > 0.f)
		{
			float FuelRate = FlightData ? FlightData->FuelConsumptionRate : 1.0f;
			float ThrustMult = FlightData ? FlightData->ThrustFuelMultiplier : 2.0f;
			float MaxThrust = FlightData ? FlightData->MaxForwardThrust : 50000.0f;

			float ThrustRatio = (MaxThrust > 0.f) ? (CurrentThrust / MaxThrust) : 0.f;
			float ActualConsumption = FuelRate + (ThrustRatio * FuelRate * ThrustMult);
			
			CurrentFuel = FMath::Max(0.f, CurrentFuel - (ActualConsumption * DeltaTime));

			// Niagara 파라미터 업데이트 (연료량 연동)
			if (EngineTrailComponent)
			{
				EngineTrailComponent->SetFloatParameter(TEXT("FuelPercentage"), GetFuelPercentage());
			}

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(2, DeltaTime, FColor::Green, 
					FString::Printf(TEXT("Fuel: %.1f%%"), GetFuelPercentage() * 100.f));
			}

			// 일반 비행 시 안정성 유지 (공기 저항)
			MeshComponent->SetLinearDamping(0.5f);
		}
		else
		{
			// 연료 고갈 시: 추진력 차단 및 엔진 트레일 비활성화
			if (CurrentThrust > 0.f)
			{
				CurrentThrust = 0.f;
				if (EngineTrailComponent) EngineTrailComponent->Deactivate();
				UE_LOG(LogWings, Warning, TEXT("OUT OF FUEL! Momentum preserved, gravity taking over."));
			}
			
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(2, DeltaTime, FColor::Red, TEXT("OUT OF FUEL!"));
			}

			// 활공을 위해 저항 극최소화 (관성 보존 극대화)
			MeshComponent->SetLinearDamping(0.01f);
		}

		// 1. Velocity Alignment: 진행 방향 보정 (엔진 유무에 따라 감도 조절)
		FVector CurrentVelocity = MeshComponent->GetPhysicsLinearVelocity();
		float Speed = CurrentVelocity.Size();
		float AlignSpeed = FlightData ? FlightData->VelocityAlignmentSpeed : 2.0f;
		
		// 연료 고갈 시에는 보정력을 약화시켜 활공 느낌 강화
		if (CurrentFuel <= 0.f) AlignSpeed *= 0.5f;

		if (Speed > 100.f)
		{
			// 현재 속도 벡터를 기체 정면 방향으로 부드럽게 회전시킴
			FVector TargetVelocity = GetActorForwardVector() * Speed;
			FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, DeltaTime, AlignSpeed);
			
			// 중요: 벡터의 방향만 취하고 원래의 속력(Magnitude)을 유지하여 에너지 손실 방지
			if (!NewVelocity.IsNearlyZero())
			{
				NewVelocity = NewVelocity.GetSafeNormal() * Speed;
			}
			
			MeshComponent->SetPhysicsLinearVelocity(NewVelocity);
		}

		// 2. Auto-Leveling (PD 제어기)
		if (bEnableAutoLeveling)
		{
			float LevelingSpeed = FlightData ? FlightData->AutoLevelingSpeed : 1.5f;
			FQuat CurrentQuat = GetActorQuat();
			FVector RightVector = CurrentQuat.GetRightVector();
			float RollError = FVector::DotProduct(RightVector, FVector::UpVector);

			FVector AngVel = MeshComponent->GetPhysicsAngularVelocityInRadians();
			float RollAngVel = FVector::DotProduct(AngVel, GetActorForwardVector());

			float P_Gain = LevelingSpeed * 40.0f; 
			float D_Gain = FMath::Sqrt(P_Gain) * 2.0f;

			float LevelingTorque = (-RollError * P_Gain) - (RollAngVel * D_Gain);
			MeshComponent->AddTorqueInRadians(GetActorForwardVector() * LevelingTorque, NAME_None, true);
		}

		// 3. Bank-to-Turn
		float BTTAmount = FlightData ? FlightData->BankToTurnAmount : 0.1f;
		FVector Right = GetActorRightVector();
		float RollLean = FVector::DotProduct(Right, FVector::UpVector);
		if (FMath::Abs(RollLean) > 0.05f)
		{
			float YawTorque = -RollLean * BTTAmount * 10.0f;
			MeshComponent->AddTorqueInRadians(FVector::UpVector * YawTorque, NAME_None, true);
		}

		// 4. Constant Thrust & Fake Lift
		if (CurrentThrust > 0.f)
		{
			MeshComponent->AddForce(GetActorForwardVector() * CurrentThrust, NAME_None, true);
		}

		// Fake Lift: 속도에 비례하여 중력의 일부 상쇄 (부유감 개선)
		float LiftMultiplier = FlightData ? FlightData->LiftForceMultiplier : 0.1f;
		
		// 연료 고갈 시에는 양력을 90% 제거하여 묵직한 다이빙 유도
		if (CurrentFuel <= 0.f) LiftMultiplier *= 0.1f;

		if (Speed > 500.f)
		{
			FVector LiftForce = FVector::UpVector * Speed * LiftMultiplier * MeshComponent->GetMass();
			MeshComponent->AddForce(LiftForce, NAME_None, false);
		}

		// 5. Dynamic Camera (FOV & Distance)
		float SpeedAlpha = FMath::Clamp(Speed / DynamicCameraSpeedThreshold, 0.f, 1.f);
		float InterpSpeed = FlightData ? FlightData->DynamicCameraInterpSpeed : 2.0f;

		float TargetFOV = FMath::Lerp(MinFOV, MaxFOV, SpeedAlpha);
		float TargetArmLength = FMath::Lerp(MinArmLength, MaxArmLength, SpeedAlpha);

		CameraComponent->SetFieldOfView(FMath::FInterpTo(CameraComponent->FieldOfView, TargetFOV, DeltaTime, InterpSpeed));
		SpringArmComponent->TargetArmLength = FMath::FInterpTo(SpringArmComponent->TargetArmLength, TargetArmLength, DeltaTime, InterpSpeed);

		// 6. Free Look Smooth Return (Quaternion Interp)
		if (!bIsFreeLooking)
		{
			FRotator CurrentRelativeRot = SpringArmComponent->GetRelativeRotation();
			if (!CurrentRelativeRot.IsNearlyZero())
			{
				float ReturnSpeed = FlightData ? FlightData->CameraReturnSpeed : 5.0f;
				FRotator NewRelativeRot = FMath::RInterpTo(CurrentRelativeRot, FRotator::ZeroRotator, DeltaTime, ReturnSpeed);
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
			EnhancedInputComponent->BindAction(InputConfig->IA_Aim, ETriggerEvent::Triggered, this, &AWingsPawnBase::Input_FlightMouse);
			EnhancedInputComponent->BindAction(InputConfig->IA_Pitch, ETriggerEvent::Triggered, this, &AWingsPawnBase::Input_PitchKeyboard);
			EnhancedInputComponent->BindAction(InputConfig->IA_Roll, ETriggerEvent::Triggered, this, &AWingsPawnBase::Input_Roll);
			EnhancedInputComponent->BindAction(InputConfig->IA_Thrust, ETriggerEvent::Triggered, this, &AWingsPawnBase::Input_Thrust);

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
		if (EngineTrailComponent) EngineTrailComponent->Activate();
		break;

	case EWingsPawnState::Crashed:
		if (EngineTrailComponent) EngineTrailComponent->Deactivate();
		break;
	}
}

void AWingsPawnBase::Input_FlightMouse(const FInputActionValue& Value)
{
	if (CurrentState != EWingsPawnState::Flying) return;

	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (bIsFreeLooking)
	{
		FRotator NewRotation = SpringArmComponent->GetRelativeRotation();
		NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + (LookAxisVector.Y * FreeLookSensitivity), -80.f, 80.f);
		NewRotation.Yaw += (LookAxisVector.X * FreeLookSensitivity);
		SpringArmComponent->SetRelativeRotation(NewRotation);
	}
	else
	{
		// 코드 내 감도 곱셈 제거 (IMC Modifier 활용 권장)
		float PitchSens = FlightData ? FlightData->PitchSensitivity : 1.0f;
		float YawSens = FlightData ? FlightData->YawSensitivity : 1.0f;

		float PitchTorque = LookAxisVector.Y * PitchSens * 25.f;
		float YawTorque = LookAxisVector.X * YawSens * 15.f;

		MeshComponent->AddTorqueInRadians(GetActorRightVector() * PitchTorque, NAME_None, true);
		MeshComponent->AddTorqueInRadians(GetActorUpVector() * YawTorque, NAME_None, true);
	}
}

void AWingsPawnBase::Input_PitchKeyboard(const FInputActionValue& Value)
{
	if (CurrentState != EWingsPawnState::Flying) return;

	float PitchValue = Value.Get<float>();
	float PitchSens = FlightData ? FlightData->PitchSensitivity : 1.0f;
	float PitchTorque = PitchValue * PitchSens * 30.f;
	MeshComponent->AddTorqueInRadians(GetActorRightVector() * PitchTorque, NAME_None, true);
}

void AWingsPawnBase::Input_Roll(const FInputActionValue& Value)
{
	if (CurrentState != EWingsPawnState::Flying) return;

	float RollValue = Value.Get<float>();
	float RollSens = FlightData ? FlightData->RollSensitivity : 1.0f;
	float SideForce = FlightData ? FlightData->FlightSideMoveForce : 1500.0f;

	float RollTorque = RollValue * RollSens * 30.f;
	MeshComponent->AddTorqueInRadians(GetActorForwardVector() * RollTorque, NAME_None, true);

	FVector SideMoveForce = GetActorRightVector() * RollValue * SideForce * 0.01f;
	MeshComponent->AddForce(SideMoveForce, NAME_None, true);
}

void AWingsPawnBase::Input_Thrust(const FInputActionValue& Value)
{
	if (CurrentState != EWingsPawnState::Flying || CurrentFuel <= 0.f) return;

	float ThrustInput = Value.Get<float>();
	float MaxThrust = FlightData ? FlightData->MaxForwardThrust : 50000.0f;
	float Step = FlightData ? FlightData->ThrustStep : 100.0f;

	CurrentThrust = FMath::Clamp(CurrentThrust + (ThrustInput * Step), 0.f, MaxThrust);
}

void AWingsPawnBase::Input_FreeLookStarted(const FInputActionValue& Value)
{
	bIsFreeLooking = true;
}

void AWingsPawnBase::Input_FreeLookCompleted(const FInputActionValue& Value)
{
	bIsFreeLooking = false;
}
