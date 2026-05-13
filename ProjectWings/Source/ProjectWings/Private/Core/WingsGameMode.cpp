// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/WingsGameMode.h"
#include "Core/WingsGameState.h"
#include "Core/WingsPlayerController.h"
#include "Pawn/WingsPawnBase.h"
#include "UI/WingsHUD.h"
#include "Environment/WingsDestructibleActor.h"
#include "EngineUtils.h"
#include "ProjectWings/ProjectWings.h"

AWingsGameMode::AWingsGameMode()
{
    // 기본 클래스 설정
    GameStateClass = AWingsGameState::StaticClass();
    PlayerControllerClass = AWingsPlayerController::StaticClass();
    DefaultPawnClass = AWingsPawnBase::StaticClass();
    
    HUDClass = AWingsHUD::StaticClass();
}

void AWingsGameMode::BeginPlay()
{
    Super::BeginPlay();

    // 월드 내 모든 목표물(Target) 카운트
    TotalTargets = 0;
    for (TActorIterator<AWingsDestructibleActor> It(GetWorld()); It; ++It)
    {
        if (It->IsTarget())
        {
            TotalTargets++;
        }
    }
    RemainingTargets = TotalTargets;

    UE_LOG(LogWings, Display, TEXT("Stage Initialized. Total Targets: %d"), TotalTargets);
}

void AWingsGameMode::OnAircraftLaunched()
{
    CurrentSpawnCount++;
    bIsWaitingForRetry = false;
    UE_LOG(LogWings, Display, TEXT("Aircraft Launched! Remaining Spawns: %d"), GetRemainingSpawns());
}

void AWingsGameMode::OnAircraftCrashed()
{
    if (bIsGameOver) return;

    UE_LOG(LogWings, Display, TEXT("Aircraft Crashed! Checking game state..."));

    if (GetRemainingSpawns() > 0)
    {
        UE_LOG(LogWings, Display, TEXT("Retries left. Waiting for R key input..."));
        bIsWaitingForRetry = true;
    }
    else
    {
        HandleGameLoss();
    }
}

void AWingsGameMode::OnTargetDestroyed()
{
    if (bIsGameOver) return;

    RemainingTargets--;
    UE_LOG(LogWings, Display, TEXT("Target Destroyed! Remaining Targets: %d"), RemainingTargets);

    if (RemainingTargets <= 0)
    {
        HandleGameWin();
    }
}

void AWingsGameMode::HandleGameWin()
{
    if (bIsGameOver) return;
    bIsGameOver = true;
    bIsWin = true;
    bIsWaitingForRetry = false;

    UE_LOG(LogWings, Display, TEXT("============================"));
    UE_LOG(LogWings, Display, TEXT("      STAGE CLEAR!!!"));
    UE_LOG(LogWings, Display, TEXT("============================"));

    // 알림 방송!
    OnGameStateChanged.Broadcast(true);
}

void AWingsGameMode::HandleGameLoss()
{
    if (bIsGameOver) return;
    bIsGameOver = true;
    bIsLoss = true;
    bIsWaitingForRetry = false;

    UE_LOG(LogWings, Display, TEXT("============================"));
    UE_LOG(LogWings, Display, TEXT("      GAME OVER..."));
    UE_LOG(LogWings, Display, TEXT("============================"));

    // 알림 방송!
    OnGameStateChanged.Broadcast(false);
}
