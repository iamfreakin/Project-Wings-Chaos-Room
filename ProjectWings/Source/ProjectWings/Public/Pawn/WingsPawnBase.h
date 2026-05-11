// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "WingsPawnBase.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UNiagaraComponent;
class UInputMappingContext;
class UWingsInputConfigData;
struct FInputActionValue;

/**
 * 기체의 현재 상태를 정의합니다.
 */
UENUM(BlueprintType)
enum class EWingsPawnState : uint8
{
	Ready UMETA(DisplayName = "Ready"),     // 발사 대기 (조준 중)
	Flying UMETA(DisplayName = "Flying"),   // 비행 중 (조종 가능)
	Crashed UMETA(DisplayName = "Crashed")  // 추락/충돌 (조종 불가)
};

/**
 * 프로젝트의 모든 비행 기체의 베이스가 되는 Pawn 클래스입니다.
 */
UCLASS()
class PROJECTWINGS_API AWingsPawnBase : public APawn
{
	GENERATED_BODY()

public:
	AWingsPawnBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	/** 상태 변경 함수 */
	void SetPawnState(EWingsPawnState NewState);

	/** 입력 처리 함수 */
	void Input_Aim(const FInputActionValue& Value);
	void Input_LaunchStarted(const FInputActionValue& Value);
	void Input_LaunchCompleted(const FInputActionValue& Value);

	/** 비행 중 입력 처리 */
	void Input_FlightMouse(const FInputActionValue& Value);
	void Input_PitchKeyboard(const FInputActionValue& Value);
	void Input_Roll(const FInputActionValue& Value);
	void Input_Thrust(const FInputActionValue& Value);

	/** 궤적 가이드라인 업데이트 */
	void UpdateTrajectory();

protected:
	/** 현재 기체의 상태 (Ready, Flying, Crashed) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|State")
	EWingsPawnState CurrentState;

	/** 입력 매핑 컨텍스트 (Enhanced Input) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** 입력 액션 데이터 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Input")
	TObjectPtr<UWingsInputConfigData> InputConfig;

	/** 조준 대기 상태(Ready)에서의 마우스 회전 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats")
	float AimRotationSpeed;

	/** 비행 중 상하(Pitch) 회전 감도 (마우스/키보드 공용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Flight")
	float FlightPitchSensitivity;

	/** 비행 중 좌우(Yaw) 회전 감도 (마우스 전용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Flight")
	float FlightYawSensitivity;

	/** 비행 중 기울기(Roll) 회전 감도 (키보드 전용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Flight")
	float FlightRollSensitivity;

	/** 진행 방향이 기체 전방을 따라가는 속도 (낮을수록 미끄러짐이 심함) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Flight")
	float VelocityAlignmentSpeed;

	/** 조작이 없을 때 자동으로 수평을 맞추는 기능 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Flight")
	bool bEnableAutoLeveling;

	/** 수평 복원 속도 (높을수록 빠르게 수평으로 돌아옴) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Flight")
	float AutoLevelingSpeed;

	/** 기체 기울기에 따른 자동 선회(Yaw) 강도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Flight")
	float BankToTurnAmount;

	/** A/D 입력 시 옆으로 밀어주는 물리적 힘의 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Flight")
	float FlightSideMoveForce;

	/** 비행 중 도달 가능한 최대 추진력 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Flight")
	float MaxForwardThrust;

	/** 추진력 조절 시 한 번에 증감하는 수치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Flight")
	float ThrustStep;

	/** 현재 기체가 유지하고 있는 추진력 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Stats|Flight")
	float CurrentThrust;

	/** 발사 시 가해질 수 있는 최대 힘 (파워 100% 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats")
	float MaxLaunchForce;

	/** 발사 파워 충전 속도 (높을수록 빠르게 충전됨) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats")
	float ChargeSpeed;

	/** 현재 충전된 발사 파워 (0.0 ~ 1.0) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Stats")
	float CurrentLaunchPower;

	/** 현재 파워 충전 중인지 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Stats")
	bool bIsCharging;

	/** 궤적 가이드라인 표시 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Trajectory")
	bool bShowTrajectory;

	/** 궤적 예측 최대 시간 (초 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Trajectory")
	float TrajectoryMaxTime;

	/** 궤적 계산 빈도 (높을수록 정밀하지만 비용 증가) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Trajectory")
	float TrajectoryFrequency;

	/** 궤적 충돌 판정 반지름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Trajectory")
	float TrajectoryRadius;

	/** 기체 물리 및 메쉬를 담당하는 루트 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** 기체와 카메라 사이의 거리를 조절하는 스프링 암 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	/** 플레이어 시점 카메라 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComponent;

	/** 비행 시 출력되는 엔진 트레일 파티클 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> EngineTrailComponent;

	/** 발사 시 가해질 최소 기본 물리적 힘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats", meta = (AllowPrivateAccess = "true"))
	float InitialLaunchForce;

public:
	/** 컴포넌트 게터 */
	UStaticMeshComponent* GetMeshComponent() const { return MeshComponent.Get(); }
	USpringArmComponent* GetSpringArmComponent() const { return SpringArmComponent.Get(); }
	UCameraComponent* GetCameraComponent() const { return CameraComponent.Get(); }
};
