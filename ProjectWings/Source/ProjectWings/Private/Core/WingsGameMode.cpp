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
    bIsWaitingForRetry = false;
    UE_LOG(LogWings, Display, TEXT("Aircraft Launched! Remaining Spawns: %d"), GetRemainingSpawns());
}

void AWingsGameMode::OnAircraftCrashed()
{
    UE_LOG(LogWings, Display, TEXT("Aircraft Crashed! Checking game state..."));

    if (GetRemainingSpawns() > 0)
    {
        UE_LOG(LogWings, Display, TEXT("Retries left. Waiting for R key input..."));
        bIsWaitingForRetry = true;
    }
    else
    {
        UE_LOG(LogWings, Warning, TEXT("NO MORE RETRIES! Game Over."));
        bIsWaitingForRetry = false;
        // Game Over logic (e.g., Show Game Over UI) could go here
    }
}
