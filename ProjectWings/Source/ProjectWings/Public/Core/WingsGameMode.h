// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WingsGameMode.generated.h"

/** 게임 상태 변경을 알리는 델리게이트 (승리 여부 전달) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, bool, bIsWin);

/**
 * 게임의 전반적인 규칙과 승리 조건을 관리하는 클래스입니다.
 */
UCLASS()
class PROJECTWINGS_API AWingsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AWingsGameMode();

    /** 기체 발사 시 호출 */
    virtual void OnAircraftLaunched();

    /** 기체 추락 시 호출 */
    virtual void OnAircraftCrashed();

    /** 목표물 파괴 시 호출 */
    void OnTargetDestroyed();

    /** 게임 상태 변경 알림판 */
    UPROPERTY(BlueprintAssignable, Category = "Wings|Events")
    FOnGameStateChanged OnGameStateChanged;

protected:
    virtual void BeginPlay() override;

    /** 승리/패배 처리 */
    void HandleGameWin();
    void HandleGameLoss();

protected:
    /** 스테이지당 허용된 총 발사 횟수 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wings|Stage")
    int32 TotalSpawnsAllowed = 3;

    /** 현재까지 발사된 기체 수 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Stage")
    int32 CurrentSpawnCount = 0;

    /** 현재 재시도 입력을 기다리는 중인지 여부 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Stage")
    bool bIsWaitingForRetry = false;

    /** 전체 목표 수 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Stage")
    int32 TotalTargets = 0;

    /** 남은 목표 수 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Stage")
    int32 RemainingTargets = 0;

    /** 게임 종료 정보 */
    bool bIsGameOver = false;

public:
    /** 게터 */
    UFUNCTION(BlueprintPure, Category = "Wings|Stage")
    int32 GetRemainingSpawns() const { return FMath::Max(0, TotalSpawnsAllowed - CurrentSpawnCount); }

    UFUNCTION(BlueprintPure, Category = "Wings|Stage")
    bool IsWaitingForRetry() const { return bIsWaitingForRetry; }

    UFUNCTION(BlueprintPure, Category = "Wings|Stage")
    int32 GetTotalTargets() const { return TotalTargets; }

    UFUNCTION(BlueprintPure, Category = "Wings|Stage")
    int32 GetRemainingTargets() const { return RemainingTargets; }

    UFUNCTION(BlueprintPure, Category = "Wings|Stage")
    bool IsGameOver() const { return bIsGameOver; }

    /** [DEPRECATED] UI 호환용 승패 판단 게터 */
    UFUNCTION(BlueprintPure, Category = "Wings|Stage")
    bool IsGameWon() const { return bIsGameOver && RemainingTargets <= 0; }

    UFUNCTION(BlueprintPure, Category = "Wings|Stage")
    bool IsGameLost() const { return bIsGameOver && RemainingTargets > 0; }
};
