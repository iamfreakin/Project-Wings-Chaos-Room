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

	/** 궤적 가이드라인 업데이트 */
	void UpdateTrajectory();

protected:
	/** 현재 기체 상태 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|State")
	EWingsPawnState CurrentState;

	/** 입력 설정 데이터 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Input")
	TObjectPtr<UWingsInputConfigData> InputConfig;

	/** 조준 시 회전 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats")
	float AimRotationSpeed;

	/** 발사 관련 변수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats")
	float MaxLaunchForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats")
	float ChargeSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Stats")
	float CurrentLaunchPower;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Stats")
	bool bIsCharging;

	/** 궤적 가이드라인 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Trajectory")
	bool bShowTrajectory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Trajectory")
	float TrajectoryMaxTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Trajectory")
	float TrajectoryFrequency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Trajectory")
	float TrajectoryRadius;

	/** 기체 물리 및 메쉬를 담당하는 루트 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** 카메라 붐 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	/** 메인 카메라 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComponent;

	/** 엔진 트레일 니아가라 시스템 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> EngineTrailComponent;

	/** 발사 시 가해질 기본 물리적 힘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats", meta = (AllowPrivateAccess = "true"))
	float InitialLaunchForce;

public:
	/** 컴포넌트 게터 */
	UStaticMeshComponent* GetMeshComponent() const { return MeshComponent.Get(); }
	USpringArmComponent* GetSpringArmComponent() const { return SpringArmComponent.Get(); }
	UCameraComponent* GetCameraComponent() const { return CameraComponent.Get(); }
};
