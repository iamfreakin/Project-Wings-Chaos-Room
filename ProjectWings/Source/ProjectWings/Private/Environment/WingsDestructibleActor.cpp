// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/WingsDestructibleActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "GeometryCollection/GeometryCollectionSimulationTypes.h"
#include "Data/WingsDestructionData.h"
#include "Core/WingsGameMode.h"

AWingsDestructibleActor::AWingsDestructibleActor()
{
	// GeometryCollectionComponent 기본 설정 최적화
	if (UGeometryCollectionComponent* GCC = GetGeometryCollectionComponent())
	{
		GCC->SetGenerateOverlapEvents(true);
		GCC->SetNotifyRigidBodyCollision(true);
		GCC->SetSimulatePhysics(true); // 기본적으로 물리 시뮬레이션 활성화
		
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

		// 파편 자동 제거 설정 (Optimization)
		// 5.3+ 버전에서는 RemovalSettings가 에셋(RestCollection)으로 이동되었습니다.
		// 런타임에 에셋의 값을 수정하려면 FGeometryCollectionEdit을 통해 접근해야 합니다.
		// [최적화] 이미 설정이 되어 있다면 불필요한 에디트를 건너뛰어 물리 불안정 방지
		if (const UGeometryCollection* RestCollectionAsset = GCC->GetRestCollection())
		{
			if (!RestCollectionAsset->bRemoveOnMaxSleep)
			{
				FGeometryCollectionEdit GCEdit = GCC->EditRestCollection(GeometryCollection::EEditUpdate::RestPhysicsDynamic);
				if (UGeometryCollection* RestCollection = GCEdit.GetRestCollection())
				{
					RestCollection->bRemoveOnMaxSleep = true;
					RestCollection->MaximumSleepTime = FVector2D(DestructionData->RemovalDuration, DestructionData->RemovalDuration + 2.0f);
					RestCollection->RemovalDuration = FVector2D(2.0f, 2.0f); // 사라지는 연출 시간 (Shrink duration)
					RestCollection->bScaleOnRemoval = true;
				}
			}
		}

		// 컴포넌트 레벨에서 제거 기능 활성화
		GCC->bAllowRemovalOnSleep = true;

		// 이벤트 바인딩
		GCC->OnChaosBreakEvent.AddDynamic(this, &AWingsDestructibleActor::OnChaosBreak);
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
