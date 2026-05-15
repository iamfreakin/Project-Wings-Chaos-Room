// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WingsGameMode.generated.h"

/** 게임 상태 변경을 알리는 델리게이트 (승리 여부 전달) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, bool, bIsWin);

/** 기체 시퀀스 변경을 알리는 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSequenceChanged);

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

    /** 기체 시퀀스에 속성 추가 */
    UFUNCTION(BlueprintCallable, Category = "Wings|Sequence")
    bool AddAttributeToSequence(EWingsAttribute Attr);

    /** 기체 시퀀스에서 특정 인덱스의 속성 제거 */
    UFUNCTION(BlueprintCallable, Category = "Wings|Sequence")
    void RemoveAttributeFromSequence(int32 Index);

    /** 시퀀스 확정 */
    UFUNCTION(BlueprintCallable, Category = "Wings|Sequence")
    void ConfirmSequence();

    /** 다음에 발사할 기체 클래스 반환 */
    UFUNCTION(BlueprintPure, Category = "Wings|Sequence")
    TSubclassOf<class AWingsPawnBase> GetNextProjectileClass() const;

    /** 현재 시퀀스 정보 반환 */
    UFUNCTION(BlueprintPure, Category = "Wings|Sequence")
    const TArray<EWingsAttribute>& GetSelectedSequence() const { return SelectedSequence; }

    /** 시퀀스가 가득 찼는지 여부 */
    UFUNCTION(BlueprintPure, Category = "Wings|Sequence")
    bool IsSequenceFull() const { return SelectedSequence.Num() >= TotalSpawnsAllowed; }

    /** 시퀀스가 확정되었는지 여부 */
    UFUNCTION(BlueprintPure, Category = "Wings|Sequence")
    bool IsSequenceConfirmed() const { return bIsSequenceConfirmed; }

    /** 게임 상태 변경 알림판 */
    UPROPERTY(BlueprintAssignable, Category = "Wings|Events")
    FOnGameStateChanged OnGameStateChanged;

    /** 기체 시퀀스 변경 알림판 */
    UPROPERTY(BlueprintAssignable, Category = "Wings|Events")
    FOnSequenceChanged OnSequenceChanged;

protected:
    virtual void BeginPlay() override;

    /** 승리/패배 처리 */
    void HandleGameWin();
    void HandleGameLoss();

protected:
    /** 스테이지당 허용된 총 발사 횟수 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wings|Stage")
    int32 TotalSpawnsAllowed = 3;

    /** 속성별 기체 클래스 매핑 */
    UPROPERTY(EditDefaultsOnly, Category = "Wings|Stage")
    TMap<EWingsAttribute, TSubclassOf<class AWingsPawnBase>> AircraftClassMap;

    /** 현재 선택된 기체 속성 시퀀스 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Stage")
    TArray<EWingsAttribute> SelectedSequence;

    /** 시퀀스 확정 여부 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Stage")
    bool bIsSequenceConfirmed = false;

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

    /** 패배 판정 유예 타이머 (마지막 기체와 마지막 타겟 파괴 간의 레이스 컨디션 해결) */
    FTimerHandle LossCheckTimerHandle;

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
