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
		
		// [복구] 물리 시뮬레이션은 켜두어야 충돌(Block)이 정상 작동함
		GCC->SetSimulatePhysics(true); 
		
		// 기본적으로 내비게이션에 영향을 주지 않도록 설정 (성능)
		GCC->SetCanEverAffectNavigation(false);
	}
}

void AWingsDestructibleActor::BeginPlay()
{
	Super::BeginPlay();

	if (UGeometryCollectionComponent* GCC = GetGeometryCollectionComponent())
	{
		// 1. [충돌 설정 강화] 기체가 통과하지 못하도록 차단
		GCC->SetCollisionProfileName(TEXT("BlockAll"));
		GCC->SetCollisionResponseToAllChannels(ECR_Block);

		// 2. [추락 방지 핵심] 중력을 아예 꺼버림
		// 물리 엔진이 블록을 깨우더라도(Wake-up), 중력이 없으면 아래로 떨어지지 않음.
		GCC->SetEnableGravity(false);

		// 3. [초기 잠금] 모든 레벨에 대해 극단적으로 높은 임계치 적용
		SetDynamicDamageThreshold(10000000000.0f);

		// 4. [운동학적 고정] Static보다 더 확실하게 외부 충격에 의한 이동을 막음
		GCC->ObjectType = EObjectStateTypeEnum::Chaos_Object_Kinematic;

		// 5. [이벤트 바인딩]
		GCC->SetNotifyBreaks(true);
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
}

void AWingsDestructibleActor::SetDynamicDamageThreshold(float NewThreshold)
{
	if (UGeometryCollectionComponent* GCC = GetGeometryCollectionComponent())
	{
		TArray<float> Thresholds;
		for(int32 i=0; i<8; ++i) Thresholds.Add(NewThreshold);
		GCC->SetDamageThreshold(Thresholds);
	}
}

void AWingsDestructibleActor::SetStateToDynamic()
{
	if (UGeometryCollectionComponent* GCC = GetGeometryCollectionComponent())
	{
		// 정당한 속성 타격 시에만 중력을 켜고 Dynamic으로 전환
		GCC->SetEnableGravity(true);
		GCC->ObjectType = EObjectStateTypeEnum::Chaos_Object_Dynamic;
		
		// 물리 시뮬레이션 재확인
		GCC->SetSimulatePhysics(true);
	}
}
