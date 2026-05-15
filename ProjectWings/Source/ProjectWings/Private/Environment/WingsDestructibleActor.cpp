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

	if (UGeometryCollectionComponent* GCC = GetGeometryCollectionComponent())
	{
		// 1. [자가 붕괴 방지] 최소 임계치 고정
		GCC->SetDamageThreshold({ 500000.0f }); 

		// 2. [앵커링] 시작 시 Sleeping 상태로 설정하여 부딪히기 전까지 공중 고정
		GCC->ObjectType = EObjectStateTypeEnum::Chaos_Object_Sleeping;

		// 3. [소멸] 데이터 에셋이 있으면 제거 시간 설정 적용
		if (DestructionData)
		{
			if (const UGeometryCollection* RestCollectionAsset = GCC->GetRestCollection())
			{
				FGeometryCollectionEdit GCEdit = GCC->EditRestCollection(GeometryCollection::EEditUpdate::RestPhysicsDynamic);
				if (UGeometryCollection* RestCollection = GCEdit.GetRestCollection())
				{
					RestCollection->bRemoveOnMaxSleep = true;
					RestCollection->MaximumSleepTime = FVector2D(DestructionData->RemovalDuration, DestructionData->RemovalDuration + 0.5f);
					RestCollection->RemovalDuration = FVector2D(1.0f, 1.0f); 
					RestCollection->bScaleOnRemoval = true;
				}
			}
		}

		GCC->SetNotifyBreaks(true);
		GCC->OnChaosBreakEvent.AddDynamic(this, &AWingsDestructibleActor::OnChaosBreak);
	}
}

void AWingsDestructibleActor::OnChaosBreak(const FChaosBreakEvent& BreakEvent)
{
	// 1. 목표물 파괴 체크 (이 로직은 유지해야 게임 클리어가 가능함)
	if (bIsTarget && !bHasBeenCounted)
	{
		if (AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>())
		{
			bHasBeenCounted = true;
			GM->OnTargetDestroyed();
		}
	}

	// 2. 파괴 전파 로직 삭제 (도미노 현상 방지)
}
