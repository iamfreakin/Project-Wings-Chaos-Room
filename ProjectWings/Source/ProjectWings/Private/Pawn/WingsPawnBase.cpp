// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/WingsPawnBase.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Core/WingsGameState.h"
#include "Core/WingsGameMode.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Data/WingsInputConfigData.h"
#include "Data/WingsFlightData.h"
#include "Data/WingsDestructionData.h"
#include "Kismet/GameplayStatics.h"
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemObjects.h"
#include "Field/FieldSystemTypes.h"
#include "Environment/WingsDestructibleActor.h"
#include "ProjectWings/ProjectWings.h"

AWingsPawnBase::AWingsPawnBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 루트 컴포넌트: 스태틱 메쉬
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetNotifyRigidBodyCollision(true);
	MeshComponent->SetMassOverrideInKg(NAME_None, 200.0f, true); 

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

	// 파괴 충격파를 발산하는 필드 시스템 컴포넌트
	FieldSystemComponent = CreateDefaultSubobject<UFieldSystemComponent>(TEXT("FieldSystemComponent"));
	FieldSystemComponent->SetupAttachment(RootComponent);

	// 초기 상태 설정
	CurrentState = EWingsPawnState::Flying;
	bIsFreeLooking = false;
}

void AWingsPawnBase::BeginPlay()
{
	Super::BeginPlay();

	// 데이터 에셋 기반 초기 설정
	if (FlightData)
	{
		if (SpringArmComponent)
		{
			SpringArmComponent->CameraLagSpeed = FlightData->CameraLagSpeed;
			SpringArmComponent->CameraRotationLagSpeed = FlightData->CameraRotationLagSpeed;
			SpringArmComponent->TargetArmLength = FlightData->MinArmLength;
		}

		if (MeshComponent)
		{
			MeshComponent->SetMassOverrideInKg(NAME_None, FlightData->PawnMass, true);
		}
	}

	if (EngineTrailComponent)
	{
		EngineTrailComponent->Activate();
	}

	// 충돌 이벤트 바인딩
	if (MeshComponent)
	{
		MeshComponent->OnComponentHit.AddDynamic(this, &AWingsPawnBase::OnMeshHit);
	}
}

void AWingsPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == EWingsPawnState::Flying)
	{
		FVector CurrentVelocity = MeshComponent->GetPhysicsLinearVelocity();
		float Speed = CurrentVelocity.Size();

		// [개선] 최고 속도 향상을 위한 동적 선형 감쇄 (고속에서 저항 감소)
		float DynamicDamping = FMath::GetMappedRangeValueClamped(FVector2D(2000.f, 15000.f), FVector2D(0.1f, 0.6f), Speed);
		MeshComponent->SetLinearDamping(DynamicDamping);

		// 1. Velocity Alignment: 진행 방향 보정 (공기 저항)
		float AlignSpeed = FlightData ? (FlightData->VelocityAlignmentSpeed * 0.6f) : 1.2f;

		if (Speed > 100.f)
		{
			FVector TargetVelocity = GetActorForwardVector() * Speed;
			FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, DeltaTime, AlignSpeed);
			
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

		// 4. Constant Thrust & Refined Lift
		if (CurrentThrust > 0.f)
		{
			// [개선] 목표 속도(400km/h) 달성을 위한 추진력 강화
			MeshComponent->AddForce(GetActorForwardVector() * CurrentThrust * 250.0f, NAME_None, false);
		}

		// [개선] 양력 계산 방식 정교화 (기각에 따른 효율 적용)
		float PitchAlpha = FVector::DotProduct(GetActorForwardVector(), FVector::UpVector); // -1(수직하강) ~ 1(수직상승)
		float LiftEffectiveness = FMath::Clamp(1.0f - FMath::Abs(PitchAlpha), 0.f, 1.f);
		float LiftMultiplier = FlightData ? (FlightData->LiftForceMultiplier * 0.5f) : 0.05f;

		if (Speed > 500.f)
		{
			FVector LiftForce = FVector::UpVector * Speed * LiftMultiplier * LiftEffectiveness * MeshComponent->GetMass();
			MeshComponent->AddForce(LiftForce, NAME_None, false);
		}

		// 5. Dynamic Camera (FOV & Distance)
		float DynamicThreshold = FlightData ? FlightData->DynamicCameraSpeedThreshold : 5000.0f;
		float SpeedAlpha = FMath::Clamp(Speed / DynamicThreshold, 0.f, 1.f);
		float InterpSpeed = FlightData ? FlightData->DynamicCameraInterpSpeed : 2.0f;

		float LocalMinFOV = FlightData ? FlightData->MinFOV : 90.0f;
		float LocalMaxFOV = FlightData ? FlightData->MaxFOV : 110.0f;
		float LocalMinArm = FlightData ? FlightData->MinArmLength : 800.0f;
		float LocalMaxArm = FlightData ? FlightData->MaxArmLength : 1200.0f;

		float TargetFOV = FMath::Lerp(LocalMinFOV, LocalMaxFOV, SpeedAlpha);
		float TargetArmLength = FMath::Lerp(LocalMinArm, LocalMaxArm, SpeedAlpha);

		CameraComponent->SetFieldOfView(FMath::FInterpTo(CameraComponent->FieldOfView, TargetFOV, DeltaTime, InterpSpeed));
		SpringArmComponent->TargetArmLength = FMath::FInterpTo(SpringArmComponent->TargetArmLength, TargetArmLength, DeltaTime, InterpSpeed);

		// 6. Free Look Smooth Return
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
	else if (CurrentState == EWingsPawnState::Crashed)
	{
		// 사망 카메라 (Death Cam) 보간: 충돌 후 서서히 뒤로 빠지며 위에서 아래를 조망하는 구도로 전환
		if (FlightData)
		{
			float DeathInterpSpeed = FlightData->DeathCamInterpSpeed;
			float LocalMaxFOV = FlightData->MaxFOV;
			
			// 1. 카메라 거리 보간: 기체로부터 설정된 거리(DeathCamDistance)만큼 부드럽게 뒤로 물러남
			SpringArmComponent->TargetArmLength = FMath::FInterpTo(SpringArmComponent->TargetArmLength, FlightData->DeathCamDistance, DeltaTime, DeathInterpSpeed);
			
			// 2. 높이 오프셋 보간: 기체 바로 위가 아닌, 약간 위쪽(DeathCamHeight)에서 내려다보는 구도 형성
			FVector CurrentOffset = SpringArmComponent->SocketOffset;
			FVector TargetOffset = FVector(0.0f, 0.0f, FlightData->DeathCamHeight);
			SpringArmComponent->SocketOffset = FMath::VInterpTo(CurrentOffset, TargetOffset, DeltaTime, DeathInterpSpeed);

			// 3. 시야각(FOV) 확장: 충격 현장을 더 넓게 볼 수 있도록 최대 FOV로 부드럽게 전환
			CameraComponent->SetFieldOfView(FMath::FInterpTo(CameraComponent->FieldOfView, LocalMaxFOV, DeltaTime, DeathInterpSpeed));

			// 4. [개선] 카메라 회전 보간: 사용자가 조작 중이 아닐 때만 부드럽게 수평 구도로 전환
			if (!bIsDeathCamInitialized)
			{
				FRotator CurrentRot = SpringArmComponent->GetComponentRotation();
				// Yaw는 0으로 강제하지 않고 현재 Yaw를 유지하여 방향감 보존
				FRotator TargetRot = FRotator(-30.0f, CurrentRot.Yaw, 0.0f);
				
				FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, DeathInterpSpeed);
				SpringArmComponent->SetWorldRotation(NewRot);

				// 목표 각도에 충분히 근접하면 자동 보정 완료 처리
				if (CurrentRot.Equals(TargetRot, 1.0f))
				{
					bIsDeathCamInitialized = true;
				}
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

		// [추가] GameMode에 추락 알림
		if (AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>())
		{
			GM->OnAircraftCrashed();
		}

		bIsDeathCamInitialized = false;

		// [사망 카메라 핵심 설정] 충돌 직후 기체가 회전하더라도 카메라 화면은 수평과 안정을 유지하게 함
		if (SpringArmComponent)
		{
			SpringArmComponent->bInheritPitch = false;
			SpringArmComponent->bInheritYaw = false;
			SpringArmComponent->bInheritRoll = false;
			SpringArmComponent->SetUsingAbsoluteRotation(true);

			// 충돌 직후 '수평(Roll)'만 즉시 보정하여 화면 뒤집힘 방지
			FRotator CurrentRot = SpringArmComponent->GetComponentRotation();
			CurrentRot.Roll = 0.0f;
			SpringArmComponent->SetWorldRotation(CurrentRot);
		}
		break;
	}
}

void AWingsPawnBase::Input_FlightMouse(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	float FreeLookSens = FlightData ? FlightData->FreeLookSensitivity : 0.5f;

	if (CurrentState == EWingsPawnState::Flying)
	{
		if (bIsFreeLooking)
		{
			FRotator NewRotation = SpringArmComponent->GetRelativeRotation();
			NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + (LookAxisVector.Y * FreeLookSens), -80.f, 80.f);
			NewRotation.Yaw += (LookAxisVector.X * FreeLookSens);
			SpringArmComponent->SetRelativeRotation(NewRotation);
		}
		else
		{
			float PitchSens = FlightData ? FlightData->PitchSensitivity : 1.0f;
			float YawSens = FlightData ? FlightData->YawSensitivity : 1.0f;

			float PitchTorque = LookAxisVector.Y * PitchSens * 25.f;
			float YawTorque = LookAxisVector.X * YawSens * 15.f;

			MeshComponent->AddTorqueInRadians(GetActorRightVector() * PitchTorque, NAME_None, true);
			MeshComponent->AddTorqueInRadians(GetActorUpVector() * YawTorque, NAME_None, true);
		}
	}
	else if (CurrentState == EWingsPawnState::Crashed)
	{
		// [사망 카메라 자유 회전] 충돌 상태에서 마우스 이동 시 기체 주위를 공전(Orbit)함
		if (SpringArmComponent)
		{
			// 플레이어가 조작을 시작하면 자동 보정 중단
			bIsDeathCamInitialized = true;

			FRotator NewRotation = SpringArmComponent->GetComponentRotation();
			NewRotation.Yaw += (LookAxisVector.X * FreeLookSens);
			// 바닥을 뚫지 않도록 Pitch 제한 (-80도 ~ 20도)
			NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + (LookAxisVector.Y * FreeLookSens), -80.f, 20.f);
			
			SpringArmComponent->SetWorldRotation(NewRotation);
		}
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
	if (CurrentState != EWingsPawnState::Flying) return;

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

UInputMappingContext* AWingsPawnBase::GetDefaultMappingContext() const
{
	return DefaultMappingContext.Get();
}

float AWingsPawnBase::GetPawnMass() const
{
	return FlightData ? FlightData->PawnMass : 200.0f;
}

void AWingsPawnBase::OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (CurrentState != EWingsPawnState::Flying) return;

	if (AWingsDestructibleActor* DestructibleActor = Cast<AWingsDestructibleActor>(OtherActor))
	{
		float DamageMultiplier = 0.01f;

		// 1. 속성 비교 및 상태 전환
		if (Attribute == EWingsAttribute::Universal || Attribute == DestructibleActor->GetAttribute())
		{
			DamageMultiplier = 1.0f;
			// 속성이 맞으면 물리 활성화 및 정상 임계치로 설정
			DestructibleActor->SetStateToDynamic();
			DestructibleActor->SetDynamicDamageThreshold(80000000.0f);
			UE_LOG(LogWings, Log, TEXT("Attribute Match! Applying Full Damage."));

			// 2. 충격량 기반 속도 역계산 (충돌 전 속도 추정)
			float ImpactSpeedKmh = (NormalImpulse.Size() / MeshComponent->GetMass()) * 0.036f;
		
			// 3. 파괴 필드 생성 (속성이 맞을 때만 필드 생성)
			SpawnDestructionField(Hit.ImpactPoint, Hit.ImpactNormal, DestructibleActor->GetDestructionData(), DamageMultiplier, ImpactSpeedKmh);
		}
		else
		{
			// 속성이 안 맞으면 임계치를 극단적으로 높여 물리 파괴를 막고, 필드도 생성하지 않음
			DestructibleActor->SetDynamicDamageThreshold(10000000000.0f);
			UE_LOG(LogWings, Warning, TEXT("Attribute Mismatch! Target LOCKED (Threshold Raised)."));
		}
	}

	SetPawnState(EWingsPawnState::Crashed);
	UE_LOG(LogWings, Log, TEXT("Pawn Crashed! Raw Velocity at Hit: %.1f"), GetVelocity().Size());
}

void AWingsPawnBase::SpawnDestructionField(FVector ContactLocation, FVector HitNormal, const UWingsDestructionData* DestructionData, float DamageMultiplier, float ImpactSpeed)
{
	if (!FieldSystemComponent || !MeshComponent || !DestructionData) return;

	// 1. 속도 체크 (전달받은 충격 속도 기준)
	float MinSpeed = DestructionData->MinBreakSpeedKmh;

	if (ImpactSpeed < MinSpeed)
	{
		UE_LOG(LogWings, Warning, TEXT("Impact too slow to break (%.1f < %.1f Km/h)"), ImpactSpeed, MinSpeed);
		return;
	}

	// 2. 파괴력 및 밀어내는 힘 설정 (상성 배율 DamageMultiplier 적용)
	float Strength = 100000000.0f * DamageMultiplier; // 단단해진 블록을 뚫기 위해 기본 강도 상향
	float PushForce = DestructionData->ExplosionForce * DamageMultiplier;
	float Radius = 600.0f * (DamageMultiplier > 0.5f ? 1.0f : 0.3f);

	// A & B: Strain 필드 (파괴 트리거)
	URadialFalloff* StrainFalloff = NewObject<URadialFalloff>(this);
	StrainFalloff->SetRadialFalloff(Strength, 0.f, Strength, 0.f, Radius, ContactLocation, EFieldFalloffType::Field_FallOff_None);
	FieldSystemComponent->ApplyPhysicsField(true, EFieldPhysicsType::Field_ExternalClusterStrain, nullptr, StrainFalloff);

	// C: 밀어내기 필드 (LinearVelocity)
	URadialVector* RadialVector = NewObject<URadialVector>(this);
	RadialVector->SetRadialVector(PushForce * 100.0f, ContactLocation); // 힘 보정

	URadialIntMask* RadialMask = NewObject<URadialIntMask>(this);
	RadialMask->SetRadialIntMask(Radius, ContactLocation, 1, 0, Field_Set_Always);

	UCullingField* CullingField = NewObject<UCullingField>(this);
	CullingField->SetCullingField(RadialMask, RadialVector, Field_Culling_Outside);

	FieldSystemComponent->ApplyPhysicsField(true, EFieldPhysicsType::Field_LinearVelocity, nullptr, CullingField);

	UE_LOG(LogWings, Log, TEXT("Break with Force! Multiplier: %.2f, Strength: %.1f, Push: %.1f"), DamageMultiplier, Strength, PushForce);
}
