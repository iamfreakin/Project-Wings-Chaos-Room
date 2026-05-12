// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/WingsDestructibleActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Data/WingsDestructionData.h"

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
		// 참고: 실제 데미지 임계값 및 스트레인 설정은 컴포넌트의 물리 설정을 통해 이루어집니다.
		
		// 예시: 파편 제거 시간 설정 (가상 로직, 실제로는 솔버 설정을 통해 제어)
		// GCC->SetCustomRemovalDuration(DestructionData->RemovalDuration);
		
		// 충돌 발생 시 데미지를 입도록 설정
		GCC->SetDamageThreshold({ DestructionData->DamageThreshold });
	}
}
