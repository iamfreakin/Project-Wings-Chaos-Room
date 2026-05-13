// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WingsGameMode.generated.h"

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

protected:
    /** 스테이지당 허용된 총 발사 횟수 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wings|Stage")
    int32 TotalSpawnsAllowed = 3;

    /** 현재까지 발사된 기체 수 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Stage")
    int32 CurrentSpawnCount = 0;

public:
    /** 게터 */
    UFUNCTION(BlueprintPure, Category = "Wings|Stage")
    int32 GetRemainingSpawns() const { return FMath::Max(0, TotalSpawnsAllowed - CurrentSpawnCount); }
};
