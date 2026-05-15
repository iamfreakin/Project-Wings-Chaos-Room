// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WingsDestructionData.generated.h"

/**
 * 파괴 가능한 물체의 데이터를 관리하는 에셋입니다. (초기화됨)
 */
UCLASS()
class PROJECTWINGS_API UWingsDestructionData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 파편이 유지되는 시간 (초). 이후에는 사라집니다. */
	UPROPERTY(EditAnywhere, Category = "Destruction|Optimization")
	float RemovalDuration = 2.0f;

	/** 파괴가 일어나기 위한 최소 기체 속도 (Km/h) */
	UPROPERTY(EditAnywhere, Category = "Destruction|Physics")
	float MinBreakSpeedKmh = 60.0f;

	/** 파괴 시 파편이 튀어나가는 힘의 세기 (밀어내기) */
	UPROPERTY(EditAnywhere, Category = "Destruction|Visual")
	float ExplosionForce = 1000.0f;
};
