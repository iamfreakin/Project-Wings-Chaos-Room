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
	float AutoLevelingSpeed = 1.5f;

	/** 뱅크-투-턴 강도 (기울기에 따른 자동 선회) */
	UPROPERTY(EditAnywhere, Category = "Flight|Physics", meta = (ToolTip = "기체가 옆으로 기울어졌을 때 자동으로 선회(Yaw) 토크를 부여하는 정도입니다."))
	float BankToTurnAmount = 0.1f;

	/** 측면 이동 힘 (Roll 시 기동성 향상) */
	UPROPERTY(EditAnywhere, Category = "Flight|Physics", meta = (ToolTip = "Roll 입력 시 기체를 해당 방향 측면으로 밀어주는 힘의 크기입니다."))
	float FlightSideMoveForce = 1500.0f;

	/** 최대 전방 추진력 */
	UPROPERTY(EditAnywhere, Category = "Flight|Thrust", meta = (ToolTip = "엔진이 낼 수 있는 최대 추진력입니다."))
	float MaxForwardThrust = 50000.0f;

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
};
