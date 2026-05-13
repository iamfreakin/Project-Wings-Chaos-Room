// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WingsUserWidget.generated.h"

class AWingsPawnBase;
class AWingsLauncher;

/**
 * 프로젝트 전용 베이스 위젯 클래스입니다.
 * 기체 및 발사대 데이터를 위젯에 전달하는 공통 로직을 포함합니다.
 */
UCLASS()
class PROJECTWINGS_API UWingsUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 현재 플레이어가 조종 중인 기체를 안전하게 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Wings|UI")
	AWingsPawnBase* GetOwningWingsPawn() const;

	/** 현재 월드에 존재하는 발사대를 반환합니다. (조준/충전 UI용) */
	UFUNCTION(BlueprintPure, Category = "Wings|UI")
	AWingsLauncher* GetActiveLauncher() const;

	/** 현재 기체의 속도를 Kmh 단위로 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Wings|UI")
	float GetCurrentSpeedKmh() const;

	/** 현재 기체의 연료 백분율을 반환합니다. (0.0 ~ 1.0) */
	UFUNCTION(BlueprintPure, Category = "Wings|UI")
	float GetFuelPercent() const;

	/** 발사대의 현재 충전 파워를 반환합니다. (0.0 ~ 1.0) */
	UFUNCTION(BlueprintPure, Category = "Wings|UI")
	float GetLaunchPowerPercent() const;

	/** "120 km/h" 형식의 포맷팅된 속도 텍스트를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Wings|UI")
	FText GetSpeedText() const;

	/** 연료 잔량에 따른 게이지 색상을 반환합니다. (Green -> Yellow -> Red) */
	UFUNCTION(BlueprintPure, Category = "Wings|UI")
	FSlateColor GetFuelColor() const;

	/** 발사대가 충전 중일 때만 UI를 표시합니다. */
	UFUNCTION(BlueprintPure, Category = "Wings|UI")
	ESlateVisibility GetLaunchPowerVisibility() const;

	/** 현재 플레이어가 비행 중일 때만 UI를 표시합니다. (속도계, 연료 게이지용) */
	UFUNCTION(BlueprintPure, Category = "Wings|UI")
	ESlateVisibility GetFlightUIVisibility() const;

	/** 플레이어가 발사대에 있을 때만 UI를 표시합니다. (조준, 파워 게이지용) */
	UFUNCTION(BlueprintPure, Category = "Wings|UI")
	ESlateVisibility GetLauncherUIVisibility() const;

	/** 현재 스테이지의 남은 기체 수를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Wings|UI")
	int32 GetRemainingSpawns() const;
	};
