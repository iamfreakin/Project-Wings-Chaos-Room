// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WingsFlightData.generated.h"

/**
 * 비행 물리 및 감도 관련 수치를 관리하는 데이터 에셋
 */
UCLASS()
class PROJECTWINGS_API UWingsFlightData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 기수의 위/아래 회전 감도 */
	UPROPERTY(EditAnywhere, Category = "Flight|Sensitivity", meta = (ToolTip = "기수의 위/아래 회전 감도입니다."))
	float PitchSensitivity = 0.05f;

	/** 기수의 좌/우 회전 감도 */
	UPROPERTY(EditAnywhere, Category = "Flight|Sensitivity", meta = (ToolTip = "기수의 좌/우 회전 감도입니다."))
	float YawSensitivity = 0.03f;

	/** 기체의 좌/우 기울기 회전 감도 */
	UPROPERTY(EditAnywhere, Category = "Flight|Sensitivity", meta = (ToolTip = "기체의 좌/우 기울기 회전 감도입니다."))
	float RollSensitivity = 0.5f;

	/** 진행 방향 정렬 속도 (미끄러짐 보정) */
	UPROPERTY(EditAnywhere, Category = "Flight|Physics", meta = (ToolTip = "기체가 진행 방향을 정면으로 맞추려는 속도입니다. 수치가 높을수록 미끄러짐이 줄어듭니다."))
	float VelocityAlignmentSpeed = 2.0f;

	/** 가짜 양력 계수 (속도 비례 중력 상쇄) */
	UPROPERTY(EditAnywhere, Category = "Flight|Physics", meta = (ToolTip = "속도에 비례하여 중력을 상쇄하는 힘의 계수입니다. 고속 비행 시 수평 유지에 도움을 줍니다."))
	float LiftForceMultiplier = 0.1f;

	/** 자동 수평 복원 속도 */
	UPROPERTY(EditAnywhere, Category = "Flight|Physics", meta = (ToolTip = "조작이 없을 때 기체를 수평 상태로 되돌리는 속도입니다."))
	float AutoLevelingSpeed = 0.5f;

	/** 뱅크-투-턴 강도 (기울기에 따른 자동 선회) */
	UPROPERTY(EditAnywhere, Category = "Flight|Physics", meta = (ToolTip = "기체가 옆으로 기울어졌을 때 자동으로 선회(Yaw) 토크를 부여하는 정도입니다."))
	float BankToTurnAmount = 0.1f;

	/** 측면 이동 힘 (Roll 시 기동성 향상) */
	UPROPERTY(EditAnywhere, Category = "Flight|Physics", meta = (ToolTip = "Roll 입력 시 기체를 해당 방향 측면으로 밀어주는 힘의 크기입니다."))
	float FlightSideMoveForce = 1500.0f;

	/** 최대 전방 추진력 */
	UPROPERTY(EditAnywhere, Category = "Flight|Thrust", meta = (ToolTip = "엔진이 낼 수 있는 최대 추진력입니다."))
	float MaxForwardThrust = 5000.0f;

	/** 추진력 가감 단위 */
	UPROPERTY(EditAnywhere, Category = "Flight|Thrust", meta = (ToolTip = "추진력을 조절할 때 한 번에 변화하는 양입니다."))
	float ThrustStep = 100.0f;

	/** 기본 연료 소모율 */
	UPROPERTY(EditAnywhere, Category = "Flight|Fuel", meta = (ToolTip = "비행 중 초당 기본적으로 소모되는 연료량입니다."))
	float FuelConsumptionRate = 1.0f;

	/** 추진력에 따른 연료 소모 배율 */
	UPROPERTY(EditAnywhere, Category = "Flight|Fuel", meta = (ToolTip = "추진력을 사용할 때 추가로 소모되는 연료의 배율입니다."))
	float ThrustFuelMultiplier = 2.0f;

	/** 동적 카메라 보간 속도 */
	UPROPERTY(EditAnywhere, Category = "Flight|Camera", meta = (ToolTip = "속도 변화에 따른 FOV 및 카메라 거리 변화가 적용되는 속도입니다."))
	float DynamicCameraInterpSpeed = 2.0f;

	/** 카메라 복귀 속도 */
	UPROPERTY(EditAnywhere, Category = "Flight|Camera", meta = (ToolTip = "자유 시점 해제 시 카메라가 원래 위치(정후방)로 돌아오는 속도입니다."))
	float CameraReturnSpeed = 5.0f;

	/** 최소 시야각 (정지/저속 시) */
	UPROPERTY(EditAnywhere, Category = "Flight|Camera", meta = (ToolTip = "최저 속도일 때의 기본 FOV입니다."))
	float MinFOV = 90.0f;

	/** 최대 시야각 (최고 속도 시) */
	UPROPERTY(EditAnywhere, Category = "Flight|Camera", meta = (ToolTip = "최고 속도일 때의 확장된 FOV입니다."))
	float MaxFOV = 110.0f;

	/** 최소 카메라 거리 (정지/저속 시) */
	UPROPERTY(EditAnywhere, Category = "Flight|Camera", meta = (ToolTip = "최저 속도일 때의 카메라와 기체 사이의 거리입니다."))
	float MinArmLength = 800.0f;

	/** 최대 카메라 거리 (최고 속도 시) */
	UPROPERTY(EditAnywhere, Category = "Flight|Camera", meta = (ToolTip = "최고 속도일 때의 카메라와 기체 사이의 거리입니다."))
	float MaxArmLength = 1200.0f;

	/** 카메라 효과가 최대로 적용되는 속도 임계값 */
	UPROPERTY(EditAnywhere, Category = "Flight|Camera", meta = (ToolTip = "이 속도에 도달했을 때 최대 FOV와 최대 거리가 적용됩니다."))
	float DynamicCameraSpeedThreshold = 5000.0f;

	/** 카메라 위치 지연(Lag) 속도 */
	UPROPERTY(EditAnywhere, Category = "Flight|Camera", meta = (ToolTip = "카메라가 기체의 위치 변화를 뒤따라가는 속도입니다. 수치가 낮을수록 부드럽게 지연됩니다."))
	float CameraLagSpeed = 10.0f;

	/** 카메라 회전 지연(Rotation Lag) 속도 */
	UPROPERTY(EditAnywhere, Category = "Flight|Camera", meta = (ToolTip = "카메라가 기체의 회전 변화를 뒤따라가는 속도입니다."))
	float CameraRotationLagSpeed = 8.0f;

	/** 자유 시점 회전 감도 */
	UPROPERTY(EditAnywhere, Category = "Flight|Camera", meta = (ToolTip = "자유 시점 모드에서 마우스 이동에 따른 카메라 회전 감도입니다."))
	float FreeLookSensitivity = 0.5f;

	/** 충격파 파괴 범위 */
	UPROPERTY(EditAnywhere, Category = "Flight|Destruction", meta = (ToolTip = "충돌 시 발생하는 충격파의 반지름입니다."))
	float DestructionFieldRadius = 500.0f;

	/** 충격파 파괴 강도 */
	UPROPERTY(EditAnywhere, Category = "Flight|Destruction", meta = (ToolTip = "충격파의 세기입니다. 장애물의 결합력을 끊는 데 사용됩니다."))
	float DestructionFieldStrength = 10000000.0f;

	/** 파괴력 계산 시 기준이 되는 질량 */
	UPROPERTY(EditAnywhere, Category = "Flight|Destruction", meta = (ToolTip = "파괴력(강도/반경) 계산 시 기준이 되는 질량입니다. 실제 질량이 이보다 크면 파괴력이 증폭됩니다."))
	float DestructionMassReference = 5000.0f;

	/** 사망 카메라 거리 */
	UPROPERTY(EditAnywhere, Category = "Flight|Camera|Death", meta = (ToolTip = "충돌 후 카메라가 기체로부터 멀어질 목표 거리입니다."))
	float DeathCamDistance = 2500.0f;

	/** 사망 카메라 높이 (Z 오프셋) */
	UPROPERTY(EditAnywhere, Category = "Flight|Camera|Death", meta = (ToolTip = "충돌 후 카메라가 위치할 높이 오프셋입니다."))
	float DeathCamHeight = 500.0f;

	/** 사망 카메라 보간 속도 */
	UPROPERTY(EditAnywhere, Category = "Flight|Camera|Death", meta = (ToolTip = "충돌 후 사망 카메라 구도로 전환되는 속도입니다."))
	float DeathCamInterpSpeed = 2.0f;
};
