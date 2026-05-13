// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/WingsDestructibleActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Data/WingsDestructionData.h"
#include "Core/WingsGameMode.h"

AWingsDestructibleActor::AWingsDestructibleActor()
{
	// GeometryCollectionComponent 기본 설정 최적화
	if (UGeometryCollectionComponent* GCC = GetGeometryCollectionComponent())
	{
		GCC->SetGenerateOverlapEvents(true);
		GCC->SetNotifyRigidBodyCollision(true);
		
		// 기본적으로 내비게이션에 영향을 주지 않도록 설정 (성능)
		GCC->SetCanEverAffectNavigation(false);
	}
}

void AWingsDestructibleActor::BeginPlay()
{
	Super::BeginPlay();

	ApplyDestructionData();
}

void AWingsDestructibleActor::ApplyDestructionData()
{
	if (!DestructionData) return;

	if (UGeometryCollectionComponent* GCC = GetGeometryCollectionComponent())
	{
		// 데이터 에셋의 수치를 카오스 컴포넌트에 적용
		GCC->SetDamageThreshold({ DestructionData->DamageThreshold });

		// 충돌 데미지 활성화 (파편이 다른 물체를 부술 수 있게 함)
		GCC->SetEnableDamageFromCollision(DestructionData->bEnableCollisionDamage);

		// 파괴 이벤트 알림 활성화 (연쇄 파괴 로직 구동용)
		GCC->SetNotifyBreaks(true);

		// 이벤트 바인딩
		GCC->OnChaosBreakEvent.AddDynamic(this, &AWingsDestructibleActor::OnChaosBreak);

		// 내부 결합력 설정 (실제 적용을 위해 필드 시스템이나 에셋 설정을 통하는 것이 정석이나, 수치 기록용)
		// GCC->SetInternalStrain(DestructionData->InternalStrain);
	}
}

void AWingsDestructibleActor::OnChaosBreak(const FChaosBreakEvent& BreakEvent)
{
	// 1. 목표물 파괴 체크
	if (bIsTarget && !bHasBeenCounted)
	{
		if (AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>())
		{
			bHasBeenCounted = true;
			GM->OnTargetDestroyed();
		}
	}

	// 2. 파괴 전파 (기존 로직)
	if (!DestructionData || !DestructionData->bEnablePropagation) return;

	if (UGeometryCollectionComponent* GCC = GetGeometryCollectionComponent())
	{
		// 파괴가 발생한 지점에서 주변으로 파괴력(Strain)을 전파하여 연쇄 반응 유도
		// PropagationDepth: 2 (한 단계 건너까지 전파)
		GCC->ApplyExternalStrain(
			BreakEvent.Index, 
			BreakEvent.Location, 
			DestructionData->PropagationRadius, 
			2, 
			1.0f, 
			DestructionData->PropagationStrength
		);

		// UE_LOG(LogWings, Verbose, TEXT("Chain Reaction! Index: %d, Location: %s"), BreakEvent.Index, *BreakEvent.Location.ToString());
	}
}
