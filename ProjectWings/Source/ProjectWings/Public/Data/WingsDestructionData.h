// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "WingsDestructionData.generated.h"

/**
 * 파괴 가능한 물체의 물리적 성질과 연출 태그를 관리하는 데이터 에셋
 */
UCLASS()
class PROJECTWINGS_API UWingsDestructionData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 파괴가 시작되는 최소 충격 강도 (Damage Threshold) */
	UPROPERTY(EditAnywhere, Category = "Destruction|Physics", meta = (ClampMin = "0.0"))
	float DamageThreshold = 5000.0f;

	/** 조각들 사이의 결합력 (Internal Strain) - 높을수록 잘 안부서짐 */
	UPROPERTY(EditAnywhere, Category = "Destruction|Physics", meta = (ClampMin = "0.0"))
	float InternalStrain = 100.0f;

	/** 파편이 유지되는 시간 (초). 이후에는 사라지도록 설정 가능 */
	UPROPERTY(EditAnywhere, Category = "Destruction|Optimization", meta = (ClampMin = "0.0"))
	float RemovalDuration = 10.0f;

	/** 파괴 시 파편이 튀어나가는 힘의 배율 */
	UPROPERTY(EditAnywhere, Category = "Destruction|Visual", meta = (ClampMin = "0.0"))
	float ExplosionForceMultiplier = 1.0f;

	/** 이 물체의 재질 태그 (예: Destruction.Material.Stone) */
	UPROPERTY(EditAnywhere, Category = "Destruction|Tags")
	FGameplayTag DestructionTag;
};
