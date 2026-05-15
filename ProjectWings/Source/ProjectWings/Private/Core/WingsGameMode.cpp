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

    // 게임 시작 시 기체 선택 단계라면 마우스 커서 표시 및 입력 모드 설정
    if (!bIsSequenceConfirmed)
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            FInputModeGameAndUI InputMode;
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = true;
        }
    }
}

bool AWingsGameMode::AddAttributeToSequence(EWingsAttribute Attr)
{
    if (bIsSequenceConfirmed || IsSequenceFull() || Attr == EWingsAttribute::None)
    {
        return false;
    }

    SelectedSequence.Add(Attr);
    OnSequenceChanged.Broadcast();
    return true;
}

void AWingsGameMode::RemoveAttributeFromSequence(int32 Index)
{
    if (bIsSequenceConfirmed || !SelectedSequence.IsValidIndex(Index))
    {
        return;
    }

    SelectedSequence.RemoveAt(Index);
    OnSequenceChanged.Broadcast();
}

void AWingsGameMode::ConfirmSequence()
{
    if (IsSequenceFull())
    {
        bIsSequenceConfirmed = true;
        
        // 발사 단계로 전환 시 입력 모드 변경
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = false;
        }
        
        OnSequenceChanged.Broadcast();
        UE_LOG(LogWings, Display, TEXT("Aircraft Sequence Confirmed. Launching phase started."));
    }
}

TSubclassOf<AWingsPawnBase> AWingsGameMode::GetNextProjectileClass() const
{
    if (SelectedSequence.IsValidIndex(CurrentSpawnCount))
    {
        EWingsAttribute CurrentAttr = SelectedSequence[CurrentSpawnCount];
        if (AircraftClassMap.Contains(CurrentAttr))
        {
            return AircraftClassMap[CurrentAttr];
        }
    }
    return nullptr;
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
        // [중요] 마지막 기체가 추락했을 때 즉시 패배를 선언하지 않고 0.15초 기다림.
        // Chaos 파괴 이벤트(OnChaosBreak)가 비동기로 발생하여 1~2프레임 뒤늦게 도착하기 때문.
        GetWorldTimerManager().SetTimer(LossCheckTimerHandle, this, &AWingsGameMode::HandleGameLoss, 0.15f, false);
    }
}

void AWingsGameMode::OnTargetDestroyed()
{
    if (bIsGameOver) return;

    RemainingTargets--;
    UE_LOG(LogWings, Display, TEXT("Target Destroyed! Remaining Targets: %d"), RemainingTargets);

    if (RemainingTargets <= 0)
    {
        // 승리 조건 달성 시 예약된 패배 판정 타이머가 있다면 취소
        GetWorldTimerManager().ClearTimer(LossCheckTimerHandle);
        HandleGameWin();
    }
}

void AWingsGameMode::HandleGameWin()
{
    if (bIsGameOver) return;
    bIsGameOver = true;
    bIsWaitingForRetry = false;

    UE_LOG(LogWings, Display, TEXT("============================"));
    UE_LOG(LogWings, Display, TEXT("      STAGE CLEAR!!!"));
    UE_LOG(LogWings, Display, TEXT("============================"));

    // 알림 방송 (승리)
    OnGameStateChanged.Broadcast(true);
}

void AWingsGameMode::HandleGameLoss()
{
    if (bIsGameOver) return;
    bIsGameOver = true;
    bIsWaitingForRetry = false;

    UE_LOG(LogWings, Display, TEXT("============================"));
    UE_LOG(LogWings, Display, TEXT("      GAME OVER..."));
    UE_LOG(LogWings, Display, TEXT("============================"));

    // 알림 방송 (패배)
    OnGameStateChanged.Broadcast(false);
}
