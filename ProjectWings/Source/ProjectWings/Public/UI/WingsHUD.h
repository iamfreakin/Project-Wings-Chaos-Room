// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WingsHUD.generated.h"

class UUserWidget;

/**
 * 게임의 메인 HUD 관리 클래스입니다.
 */
UCLASS()
class PROJECTWINGS_API AWingsHUD : public AHUD
{
	GENERATED_BODY()

public:
	AWingsHUD();

protected:
	virtual void BeginPlay() override;

    /** 게임 종료 이벤트 핸들러 */
    UFUNCTION()
    void HandleGameStateChanged(bool bIsWin);

public:
	/** 현재 생성된 메인 HUD 위젯 인스턴스 반환 */
	UFUNCTION(BlueprintPure, Category = "Wings|UI")
	UUserWidget* GetMainHUDWidget() const { return MainHUDWidget; }

protected:
	/** 에디터에서 할당할 위젯 클래스들 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|UI")
	TSubclassOf<UUserWidget> MainHUDWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|UI")
    TSubclassOf<UUserWidget> VictoryWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|UI")
    TSubclassOf<UUserWidget> GameOverWidgetClass;

	/** 생성된 위젯 인스턴스 보관 */
	UPROPERTY()
	TObjectPtr<UUserWidget> MainHUDWidget;
};
