// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "WingsGameState.generated.h"

/**
 * 전역 게임 상태를 관리하는 클래스입니다.
 * 풍향 데이터 등 모든 액터가 공유하는 정보를 담습니다.
 */
UCLASS()
class PROJECTWINGS_API AWingsGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AWingsGameState();

	/** 전역 풍향 벡터 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Environment", meta = (ToolTip = "전역적으로 적용되는 풍향 및 강도입니다."))
	FVector GlobalWindVector;
	
};
