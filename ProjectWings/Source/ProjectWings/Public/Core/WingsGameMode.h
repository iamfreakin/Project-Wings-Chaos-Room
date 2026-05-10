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
	
};
