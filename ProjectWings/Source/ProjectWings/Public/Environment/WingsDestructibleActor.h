// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "Chaos/ChaosGameplayEventDispatcher.h"
#include "ProjectWings/ProjectWings.h"
#include "WingsDestructibleActor.generated.h"

class UWingsDestructionData;

/**
 * 모든 파괴 가능한 환경 요소의 베이스 클래스입니다.
 * GeometryCollectionActor를 상속받아 Chaos Physics 최적화 기능을 활용합니다.
 */
UCLASS()
class PROJECTWINGS_API AWingsDestructibleActor : public AGeometryCollectionActor
{
	GENERATED_BODY()

public:
	AWingsDestructibleActor();

protected:
	virtual void BeginPlay() override;

protected:
	/** 파괴 물리 및 소멸 설정을 담은 데이터 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Destruction")
	TObjectPtr<UWingsDestructionData> DestructionData;

	/** 오브젝트의 재질 속성 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Destruction")
	EWingsAttribute Attribute = EWingsAttribute::None;

	/** 이 블록을 파괴하는 것이 스테이지 목표인지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Destruction")
	bool bIsTarget = false;

	/** 이미 파괴되어 목표 카운트에서 제외되었는지 여부 */
	bool bHasBeenCounted = false;

	/** Chaos 파괴 이벤트 핸들러 */
	UFUNCTION()
	void OnChaosBreak(const FChaosBreakEvent& BreakEvent);

public:
    UFUNCTION(BlueprintPure, Category = "Wings|Destruction")
    bool IsTarget() const { return bIsTarget; }

    UFUNCTION(BlueprintPure, Category = "Wings|Destruction")
    EWingsAttribute GetAttribute() const { return Attribute; }

	UFUNCTION(BlueprintPure, Category = "Wings|Destruction")
	const UWingsDestructionData* GetDestructionData() const { return DestructionData.Get(); }
};
