// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WingsPlayerController.generated.h"

/**
 * 플레이어의 입력을 관리하고 조종 상태를 스위칭하는 클래스입니다.
 */
UCLASS()
class PROJECTWINGS_API AWingsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWingsPlayerController();

protected:
    virtual void BeginPlay() override;

public:
    /** 발사 시 기체로 조종권을 넘기는 함수 */
    void TransitionToFlight(APawn* FlightPawn, APawn* PreviousPawn = nullptr);

    /** 발사대로 조종권을 다시 돌리는 함수 (재시도용) */
    void ReturnToLauncher();
	
};
