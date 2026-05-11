// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/WingsPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Launcher/WingsLauncher.h"
#include "Pawn/WingsPawnBase.h"
#include "EnhancedInputSubsystems.h"

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

void AWingsPlayerController::TransitionToFlight(APawn* FlightPawn, APawn* PreviousPawn)
{
    if (FlightPawn)
    {
        // 1. 기존 Pawn(Launcher)의 입력 컨텍스트 제거
        if (PreviousPawn)
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
            {
                if (AWingsLauncher* Launcher = Cast<AWingsLauncher>(PreviousPawn))
                {
                    Subsystem->RemoveMappingContext(Launcher->GetLauncherMappingContext());
                }
            }
        }

        // 2. 조종권 변경
        Possess(FlightPawn);

        // 3. 새로운 Pawn(FlightPawn)의 입력 컨텍스트 추가
        if (AWingsPawnBase* WingsPawn = Cast<AWingsPawnBase>(FlightPawn))
        {
             if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
             {
                 Subsystem->AddMappingContext(WingsPawn->GetDefaultMappingContext(), 0);
             }
        }
    }
}
