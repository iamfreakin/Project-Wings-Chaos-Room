// Fill out your copyright notice in the Description page of Project Settings.


#include "Launcher/WingsLauncher.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Data/WingsInputConfigData.h"
#include "Pawn/WingsPawnBase.h"
#include "Core/WingsPlayerController.h"
#include "Core/WingsGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectWings/ProjectWings.h"

AWingsLauncher::AWingsLauncher()
{
	PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    SetRootComponent(RootSceneComponent);

    LauncherMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LauncherMesh"));
    LauncherMesh->SetupAttachment(RootSceneComponent);

    LaunchDirectionIndicator = CreateDefaultSubobject<UArrowComponent>(TEXT("LaunchDirectionIndicator"));
    LaunchDirectionIndicator->SetupAttachment(LauncherMesh);

    // 카메라 시스템 초기화
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
    SpringArmComponent->SetupAttachment(LauncherMesh); // 발사대 회전을 따라가도록 메시에 부착
    SpringArmComponent->TargetArmLength = DefaultArmLength;
    SpringArmComponent->SocketOffset = DefaultSocketOffset;
    SpringArmComponent->bEnableCameraLag = true;
    SpringArmComponent->bEnableCameraRotationLag = true;
    SpringArmComponent->CameraLagSpeed = 10.0f;
    SpringArmComponent->CameraRotationLagSpeed = 8.0f;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(SpringArmComponent);
}

void AWingsLauncher::BeginPlay()
{
	Super::BeginPlay();

    // 초기 카메라 위치 설정
    if (SpringArmComponent)
    {
        SpringArmComponent->TargetArmLength = DefaultArmLength;
        SpringArmComponent->SocketOffset = DefaultSocketOffset;
    }

    // Launcher 전용 입력 컨텍스트 추가
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (LauncherMappingContext)
            {
                Subsystem->AddMappingContext(LauncherMappingContext, 0);
            }
        }
    }
}

void AWingsLauncher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (bIsCharging)
    {
        CurrentLaunchPower = FMath::Clamp(CurrentLaunchPower + (DeltaTime * ChargeSpeed), 0.f, 1.f);
    }

    UpdateCameraEffects(DeltaTime);
    UpdateTrajectory();
}

void AWingsLauncher::UpdateCameraEffects(float DeltaTime)
{
    if (!SpringArmComponent || !CameraComponent) return;

    // 1. Zoom Effect (충전량에 따라 가까워짐)
    float TargetArmLength = bIsCharging ? 
        FMath::Lerp(DefaultArmLength, ChargeZoomArmLength, CurrentLaunchPower) : 
        DefaultArmLength;
    
    SpringArmComponent->TargetArmLength = FMath::FInterpTo(SpringArmComponent->TargetArmLength, TargetArmLength, DeltaTime, 5.0f);

    // 2. Shake Effect (충전량에 따라 미세하게 떨림)
    if (bIsCharging && CurrentLaunchPower > 0.1f)
    {
        float ShakeAmount = CurrentLaunchPower * MaxShakeIntensity;
        float Time = GetWorld()->GetTimeSeconds();

        FVector ShakeOffset;
        ShakeOffset.X = FMath::Sin(Time * ShakeFrequency) * ShakeAmount;
        ShakeOffset.Y = FMath::Cos(Time * ShakeFrequency * 0.9f) * ShakeAmount;
        ShakeOffset.Z = FMath::Sin(Time * ShakeFrequency * 1.1f) * ShakeAmount;

        CameraComponent->SetRelativeLocation(ShakeOffset);
    }
    else
    {
        // 충전 중이 아니면 원래 위치로 복귀
        CameraComponent->SetRelativeLocation(FMath::VInterpTo(CameraComponent->GetRelativeLocation(), FVector::ZeroVector, DeltaTime, 10.0f));
    }
}

void AWingsLauncher::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (InputConfig)
        {
            EnhancedInputComponent->BindAction(InputConfig->IA_Aim, ETriggerEvent::Triggered, this, &AWingsLauncher::Input_Aim);
            EnhancedInputComponent->BindAction(InputConfig->IA_Launch, ETriggerEvent::Started, this, &AWingsLauncher::Input_LaunchStarted);
            EnhancedInputComponent->BindAction(InputConfig->IA_Launch, ETriggerEvent::Completed, this, &AWingsLauncher::Input_LaunchCompleted);
        }
    }
}

void AWingsLauncher::Input_Aim(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    AimRotation.Pitch = FMath::Clamp(AimRotation.Pitch + LookAxisVector.Y, -45.f, 45.f);
    AimRotation.Yaw += LookAxisVector.X;

    if (LauncherMesh)
    {
        LauncherMesh->SetRelativeRotation(AimRotation);
    }
}

void AWingsLauncher::Input_LaunchStarted(const FInputActionValue& Value)
{
    bIsCharging = true;
    CurrentLaunchPower = 0.f;
}

void AWingsLauncher::Input_LaunchCompleted(const FInputActionValue& Value)
{
    bIsCharging = false;
    
    if (ProjectileClass && LaunchDirectionIndicator)
    {
        FVector SpawnLocation = LaunchDirectionIndicator->GetComponentLocation();
        FRotator SpawnRotation = LaunchDirectionIndicator->GetComponentRotation();

        // 1. SpawnActorDeferred를 사용하여 초기화 제어권을 가짐
        if (AWingsPawnBase* LaunchedPawn = GetWorld()->SpawnActorDeferred<AWingsPawnBase>(
            ProjectileClass, 
            FTransform(SpawnRotation, SpawnLocation), 
            this, 
            GetInstigator(), 
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
        {
            // 2. 물리 및 상태 초기화 (FinishSpawning 호출 전)
            LaunchedPawn->SetPawnState(EWingsPawnState::Flying);

            // 3. 생성 완료
            UGameplayStatics::FinishSpawningActor(LaunchedPawn, FTransform(SpawnRotation, SpawnLocation));

            // 4. 기체 발사 처리 (임펄스 부여 - 생성 완료 후 수행)
            float FinalForce = MaxLaunchForce * CurrentLaunchPower;
            FVector LaunchVelocity = LaunchDirectionIndicator->GetForwardVector() * FinalForce;
            
            if (UStaticMeshComponent* Mesh = LaunchedPawn->GetMeshComponent())
            {
                Mesh->AddImpulse(LaunchVelocity, NAME_None, true);
            }

            // [추가] GameMode에 발사 알림
            if (AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>())
            {
                GM->OnAircraftLaunched();
            }

            // 5. 조종권 전환
            if (AWingsPlayerController* PC = Cast<AWingsPlayerController>(GetController()))
            {
                PC->TransitionToFlight(LaunchedPawn, this);
            }

            UE_LOG(LogWings, Display, TEXT("Launched Pawn with Power: %.2f"), CurrentLaunchPower);
        }
    }
    
    CurrentLaunchPower = 0.f;
}

void AWingsLauncher::UpdateTrajectory()
{
    if (!LaunchDirectionIndicator || !ProjectileClass) return;

    FVector StartLocation = LaunchDirectionIndicator->GetComponentLocation();
    FVector LaunchVelocity = LaunchDirectionIndicator->GetForwardVector() * (MaxLaunchForce * CurrentLaunchPower);
    
    // 발사될 기체의 CDO로부터 질량 정보를 가져옴
    float ProjectileMass = 200.0f;
    if (AWingsPawnBase* DefaultPawn = ProjectileClass->GetDefaultObject<AWingsPawnBase>())
    {
        ProjectileMass = DefaultPawn->GetPawnMass();
    }

    FVector Gravity = FVector(0.f, 0.f, GetWorld()->GetGravityZ());

    FPredictProjectilePathParams PathParams(TrajectoryRadius, StartLocation, LaunchVelocity / ProjectileMass, TrajectoryMaxTime);
    PathParams.DrawDebugTime = 0.1f;
    PathParams.DrawDebugType = EDrawDebugTrace::ForOneFrame;
    PathParams.OverrideGravityZ = Gravity.Z;
    PathParams.bTraceWithCollision = true;

    FPredictProjectilePathResult PathResult;
    UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
}
