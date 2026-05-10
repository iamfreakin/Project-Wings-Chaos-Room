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
	
};
