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

	// 1. Stats (상태 관련) 기본값 설정
	AimRotationSpeed = 2.0f;
	MaxLaunchForce = 10000.0f;
	ChargeSpeed = 0.2f;
	CurrentLaunchPower = 0.0f;
	bIsCharging = false;
	InitialLaunchForce = 1000.0f;

	// 2. Flight (비행 관련) 기본값 설정
	FlightPitchSensitivity = 0.05f;
	FlightYawSensitivity = 0.03f;
	FlightRollSensitivity = 0.5f;
	VelocityAlignmentSpeed = 2.0f;
	bEnableAutoLeveling = true;
	AutoLevelingSpeed = 1.5f;
	BankToTurnAmount = 0.5f;
	FlightSideMoveForce = 1500.0f;
	MaxForwardThrust = 50000.0f;
	ThrustStep = 100.0f;
	CurrentThrust = 0.0f;

	// 3. Trajectory (궤적 관련) 기본값 설정
	bShowTrajectory = true;
	TrajectoryMaxTime = 3.0f;
	TrajectoryFrequency = 15.0f;
	TrajectoryRadius = 10.0f;

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

	// 조준 및 충전 중일 때 궤적 업데이트
	if (CurrentState == EWingsPawnState::Ready)
	{
		UpdateTrajectory();

		// 파워 충전 로직
		if (bIsCharging)
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
	else if (CurrentState == EWingsPawnState::Flying)
	{
		// 1. Velocity Alignment: 진행 방향 보정 (속도가 낮을 때 튀지 않도록 문턱값 상향)
		FVector CurrentVelocity = MeshComponent->GetPhysicsLinearVelocity();
		float Speed = CurrentVelocity.Size();
		if (Speed > 100.f)
		{
			FVector TargetVelocity = GetActorForwardVector() * Speed;
			// 가속도 기반으로 속도 방향을 정렬
			FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, DeltaTime, VelocityAlignmentSpeed);
			MeshComponent->SetPhysicsLinearVelocity(NewVelocity);
		}

		// 2. 정밀한 물리 기반 Auto-Leveling (PD 제어기 적용)
		if (bEnableAutoLeveling)
		{
			// 기체의 현재 회전 상태 (Quaternion)
			FQuat CurrentQuat = GetActorQuat();
			
			// 기체의 우측 벡터가 월드의 Up 벡터와 얼마나 차이 나는지 계산 (Roll 오차)
			FVector RightVector = CurrentQuat.GetRightVector();
			float RollError = FVector::DotProduct(RightVector, FVector::UpVector);

			// 현재 전방축 기준 회전 속도 (D 성분: 감쇠를 위함)
			FVector AngVel = MeshComponent->GetPhysicsAngularVelocityInRadians();
			float RollAngVel = FVector::DotProduct(AngVel, GetActorForwardVector());

			// P_Gain: 돌아가려는 힘, D_Gain: 멈추려는 저항 (감쇠)
			float P_Gain = AutoLevelingSpeed * 40.0f; 
			float D_Gain = FMath::Sqrt(P_Gain) * 2.0f; // 임계 감쇠(Critical Damping)

			// PD 제어 공식: Torque = (-Error * P) - (Velocity * D)
			float LevelingTorque = (-RollError * P_Gain) - (RollAngVel * D_Gain);
			
			MeshComponent->AddTorqueInRadians(GetActorForwardVector() * LevelingTorque, NAME_None, true);
		}

		// 3. Bank-to-Turn: 기체가 기울어진 방향으로 자동으로 선회(Yaw) 토크 부여
		FVector Right = GetActorRightVector();
		float RollLean = FVector::DotProduct(Right, FVector::UpVector);
		if (FMath::Abs(RollLean) > 0.05f)
		{
			// Roll 기울기에 비례하여 월드 Up 축 기준으로 Yaw 토크 부여
			float YawTorque = -RollLean * BankToTurnAmount * 10.0f;
			MeshComponent->AddTorqueInRadians(FVector::UpVector * YawTorque, NAME_None, true);
		}

		// 4. Constant Thrust: 현재 유지 중인 추진력 적용 (가속도로 적용)
		if (CurrentThrust > 0.f)
		{
			MeshComponent->AddForce(GetActorForwardVector() * CurrentThrust, NAME_None, true);
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
			// Aim & Flight Mouse (Axis2D)
			EnhancedInputComponent->BindAction(InputConfig->IA_Aim, ETriggerEvent::Triggered, this, &AWingsPawnBase::Input_Aim);
			EnhancedInputComponent->BindAction(InputConfig->IA_Aim, ETriggerEvent::Triggered, this, &AWingsPawnBase::Input_FlightMouse);

			// Launch (Started: 충전 시작 / Completed: 발사)
			EnhancedInputComponent->BindAction(InputConfig->IA_Launch, ETriggerEvent::Started, this, &AWingsPawnBase::Input_LaunchStarted);
			EnhancedInputComponent->BindAction(InputConfig->IA_Launch, ETriggerEvent::Completed, this, &AWingsPawnBase::Input_LaunchCompleted);

			// Flight Controls
			EnhancedInputComponent->BindAction(InputConfig->IA_Pitch, ETriggerEvent::Triggered, this, &AWingsPawnBase::Input_PitchKeyboard);
			EnhancedInputComponent->BindAction(InputConfig->IA_Roll, ETriggerEvent::Triggered, this, &AWingsPawnBase::Input_Roll);
			EnhancedInputComponent->BindAction(InputConfig->IA_Thrust, ETriggerEvent::Triggered, this, &AWingsPawnBase::Input_Thrust);
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
		// bVelChange=true 옵션을 사용하여 질량과 무관하게 즉각적인 속도 변화 유도
		MeshComponent->AddImpulse(LaunchDirection * FinalForce, NAME_None, true);

		UE_LOG(LogWings, Display, TEXT("AWingsPawnBase: Launched with power %.2f (Total Force: %f)"), CurrentLaunchPower, FinalForce);
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(2, 2.f, FColor::Cyan, FString::Printf(TEXT("Launched! Power: %.2f"), CurrentLaunchPower));
		}
	}
}

void AWingsPawnBase::UpdateTrajectory()
{
	if (!bShowTrajectory || CurrentState != EWingsPawnState::Ready) return;

	// 현재 조준 방향과 충전 파워를 기반으로 초기 속도 계산
	FVector LaunchDirection = GetActorForwardVector();
	float FinalForce = FMath::Max(MaxLaunchForce * CurrentLaunchPower, InitialLaunchForce);
	FVector LaunchVelocity = LaunchDirection * FinalForce;

	FPredictProjectilePathParams PathParams;
	PathParams.StartLocation = GetActorLocation();
	PathParams.LaunchVelocity = LaunchVelocity;
	PathParams.bTraceWithCollision = true;
	PathParams.ProjectileRadius = TrajectoryRadius;
	PathParams.MaxSimTime = TrajectoryMaxTime;
	PathParams.bTraceWithChannel = true;
	PathParams.TraceChannel = ECollisionChannel::ECC_WorldStatic;
	PathParams.ActorsToIgnore.Add(this);
	
	// 디버그 라인 설정
	PathParams.DrawDebugType = EDrawDebugTrace::ForOneFrame;
	PathParams.SimFrequency = TrajectoryFrequency;

	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
}

void AWingsPawnBase::Input_FlightMouse(const FInputActionValue& Value)
{
	if (CurrentState != EWingsPawnState::Flying) return;

	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// bAccelChange = true 이므로 질량을 무시하고 직접적인 가속도를 가함
	float PitchTorque = LookAxisVector.Y * FlightPitchSensitivity * 25.f;
	float YawTorque = LookAxisVector.X * FlightYawSensitivity * 15.f;

	MeshComponent->AddTorqueInRadians(GetActorRightVector() * PitchTorque, NAME_None, true);
	MeshComponent->AddTorqueInRadians(GetActorUpVector() * YawTorque, NAME_None, true);
}

void AWingsPawnBase::Input_PitchKeyboard(const FInputActionValue& Value)
{
	if (CurrentState != EWingsPawnState::Flying) return;

	float PitchValue = Value.Get<float>();
	
	// 키보드(W/S) 입력을 통한 Pitch 제어
	float PitchTorque = PitchValue * FlightPitchSensitivity * 30.f;
	MeshComponent->AddTorqueInRadians(GetActorRightVector() * PitchTorque, NAME_None, true);
}

void AWingsPawnBase::Input_Roll(const FInputActionValue& Value)
{
	if (CurrentState != EWingsPawnState::Flying) return;

	float RollValue = Value.Get<float>();

	// 1. Roll 토크 적용
	float RollTorque = RollValue * FlightRollSensitivity * 30.f;
	MeshComponent->AddTorqueInRadians(GetActorForwardVector() * RollTorque, NAME_None, true);

	// 2. 옆으로 미는 힘(Lateral Force) 적용 (가속도로 적용)
	FVector SideMoveForce = GetActorRightVector() * RollValue * FlightSideMoveForce * 0.01f;
	MeshComponent->AddForce(SideMoveForce, NAME_None, true);
}

void AWingsPawnBase::Input_Thrust(const FInputActionValue& Value)
{
	if (CurrentState != EWingsPawnState::Flying) return;

	float ThrustInput = Value.Get<float>();
	
	// 추진력 조절 (가속도 수치로 조절됨)
	CurrentThrust = FMath::Clamp(CurrentThrust + (ThrustInput * ThrustStep), 0.f, MaxForwardThrust);
}
