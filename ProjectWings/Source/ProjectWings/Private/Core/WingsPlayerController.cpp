// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/WingsPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Launcher/WingsLauncher.h"
#include "Pawn/WingsPawnBase.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Data/WingsInputConfigData.h"
#include "Core/WingsGameMode.h"
#include "ProjectWings/ProjectWings.h"

AWingsPlayerController::AWingsPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}

void AWingsPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 마우스 커서를 숨기고 게임 입력에만 집중하도록 설정
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;

    // 월드에서 Launcher를 찾아 조종
    TArray<AActor*> FoundLaunchers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWingsLauncher::StaticClass(), FoundLaunchers);

    if (FoundLaunchers.Num() > 0)
    {
        if (AWingsLauncher* Launcher = Cast<AWingsLauncher>(FoundLaunchers[0]))
        {
            Possess(Launcher);
        }
    }
}

void AWingsPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (InputConfig && InputConfig->IA_Retry)
        {
            EnhancedInputComponent->BindAction(InputConfig->IA_Retry, ETriggerEvent::Started, this, &AWingsPlayerController::Input_Retry);
        }
    }
}

void AWingsPlayerController::Input_Retry()
{
    if (AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>())
    {
        if (GM->IsWaitingForRetry())
        {
            UE_LOG(LogWings, Display, TEXT("Retry Input Received."));
            ReturnToLauncher();
        }
    }
}

void AWingsPlayerController::TransitionToFlight(APawn* FlightPawn, APawn* PreviousPawn)
{
    if (!FlightPawn)
    {
        UE_LOG(LogWings, Error, TEXT("TransitionToFlight failed: FlightPawn is null!"));
        return;
    }

    // 1. 기존 Pawn(Launcher)의 입력 컨텍스트 제거
    if (PreviousPawn)
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            if (AWingsLauncher* Launcher = Cast<AWingsLauncher>(PreviousPawn))
            {
                Subsystem->RemoveMappingContext(Launcher->GetLauncherMappingContext());
                UE_LOG(LogWings, Display, TEXT("Removed Launcher Mapping Context."));
            }
        }
    }

    // 2. 조종권 변경
    Possess(FlightPawn);
    UE_LOG(LogWings, Display, TEXT("Possessed Flight Pawn: %s"), *FlightPawn->GetName());

    // 3. 새로운 Pawn(FlightPawn)의 입력 컨텍스트 추가
    if (AWingsPawnBase* WingsPawn = Cast<AWingsPawnBase>(FlightPawn))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(WingsPawn->GetDefaultMappingContext(), 0);
            UE_LOG(LogWings, Display, TEXT("Added Pawn Mapping Context."));
        }
    }
}

void AWingsPlayerController::ReturnToLauncher()
{
    // 현재 조종 중인 Pawn의 입력 컨텍스트 제거
    if (APawn* CurrentPawn = GetPawn())
    {
        if (AWingsPawnBase* WingsPawn = Cast<AWingsPawnBase>(CurrentPawn))
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
            {
                Subsystem->RemoveMappingContext(WingsPawn->GetDefaultMappingContext());
            }
        }
    }

    // 월드에서 Launcher를 찾아 다시 조종
    TArray<AActor*> FoundLaunchers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWingsLauncher::StaticClass(), FoundLaunchers);

    if (FoundLaunchers.Num() > 0)
    {
        if (AWingsLauncher* Launcher = Cast<AWingsLauncher>(FoundLaunchers[0]))
        {
            Possess(Launcher);
            
            // Launcher 입력 컨텍스트 다시 추가
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
            {
                Subsystem->AddMappingContext(Launcher->GetLauncherMappingContext(), 0);
            }

            UE_LOG(LogWings, Display, TEXT("Returned to Launcher."));
        }
    }
}
