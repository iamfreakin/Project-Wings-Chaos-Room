// Fill out your copyright notice in the Description page of Project Settings.


#include "Launcher/WingsLauncher.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Data/WingsInputConfigData.h"
#include "Pawn/WingsPawnBase.h"
#include "Core/WingsPlayerController.h"
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
}

void AWingsLauncher::BeginPlay()
{
	Super::BeginPlay();

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
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(1, DeltaTime, FColor::Yellow, 
                FString::Printf(TEXT("Launcher Charging: %.2f"), CurrentLaunchPower));
        }
    }

    UpdateTrajectory();
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

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();

        if (AWingsPawnBase* LaunchedPawn = GetWorld()->SpawnActor<AWingsPawnBase>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams))
        {
            // 기체 발사 처리 (임펄스 부여)
            float FinalForce = MaxLaunchForce * CurrentLaunchPower;
            FVector LaunchVelocity = LaunchDirectionIndicator->GetForwardVector() * FinalForce;
            
            if (UStaticMeshComponent* Mesh = LaunchedPawn->GetMeshComponent())
            {
                Mesh->AddImpulse(LaunchVelocity, NAME_None, true);
            }

            // 조종권 전환
            if (AWingsPlayerController* PC = Cast<AWingsPlayerController>(GetController()))
            {
                PC->TransitionToFlight(LaunchedPawn, this);
            }
        }
    }
    
    CurrentLaunchPower = 0.f;
}

void AWingsLauncher::UpdateTrajectory()
{
    if (!LaunchDirectionIndicator) return;

    FVector StartLocation = LaunchDirectionIndicator->GetComponentLocation();
    FVector LaunchVelocity = LaunchDirectionIndicator->GetForwardVector() * (MaxLaunchForce * CurrentLaunchPower);
    
    float ProjectileMass = 1.0f; 
    FVector Gravity = FVector(0.f, 0.f, GetWorld()->GetGravityZ());

    FPredictProjectilePathParams PathParams(TrajectoryRadius, StartLocation, LaunchVelocity / ProjectileMass, TrajectoryMaxTime);
    PathParams.DrawDebugTime = 0.1f;
    PathParams.DrawDebugType = EDrawDebugTrace::ForOneFrame;
    PathParams.OverrideGravityZ = Gravity.Z;
    PathParams.bTraceWithCollision = true;

    FPredictProjectilePathResult PathResult;
    UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
}
