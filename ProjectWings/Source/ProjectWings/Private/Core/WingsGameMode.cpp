// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/WingsGameMode.h"
#include "Core/WingsGameState.h"
#include "Core/WingsPlayerController.h"
#include "Pawn/WingsPawnBase.h"
#include "UI/WingsHUD.h"
#include "ProjectWings/ProjectWings.h"

AWingsGameMode::AWingsGameMode()
{
	// 기본 클래스 설정
	GameStateClass = AWingsGameState::StaticClass();
	PlayerControllerClass = AWingsPlayerController::StaticClass();
	DefaultPawnClass = AWingsPawnBase::StaticClass();
	HUDClass = AWingsHUD::StaticClass();
}

void AWingsGameMode::OnAircraftLaunched()
{
    CurrentSpawnCount++;
    UE_LOG(LogWings, Display, TEXT("Aircraft Launched! Remaining Spawns: %d"), GetRemainingSpawns());
}

void AWingsGameMode::OnAircraftCrashed()
{
    UE_LOG(LogWings, Display, TEXT("Aircraft Crashed! Checking game state..."));

    if (GetRemainingSpawns() > 0)
    {
        UE_LOG(LogWings, Display, TEXT("Retries left. Returning to Launcher in 3 seconds..."));
        
        // 3초 후 발사대로 복귀 (사망 카메라 감상 시간 제공)
        FTimerHandle RetryTimerHandle;
        GetWorldTimerManager().SetTimer(RetryTimerHandle, [this]()
        {
            if (AWingsPlayerController* PC = Cast<AWingsPlayerController>(GetWorld()->GetFirstPlayerController()))
            {
                PC->ReturnToLauncher();
            }
        }, 3.0f, false);
    }
    else
    {
        UE_LOG(LogWings, Warning, TEXT("NO MORE RETRIES! Game Over."));
        // 
    }
}
