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
class UWingsFlightData;
struct FInputActionValue;

/**
 * 기체의 현재 상태를 정의합니다. (Ready 상태 제거)
 */
UENUM(BlueprintType)
enum class EWingsPawnState : uint8
{
	Flying UMETA(DisplayName = "Flying"),   // 비행 중 (조종 가능)
	Crashed UMETA(DisplayName = "Crashed")  // 추락/충돌 (조종 불가)
};

/**
 * 프로젝트의 모든 비행 기체의 베이스가 되는 Pawn 클래스입니다.
 * 이제 발사 로직은 AWingsLauncher에서 담당합니다.
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

public:
	/** 상태 변경 함수 */
	void SetPawnState(EWingsPawnState NewState);

protected:

	/** 비행 중 입력 처리 */
	void Input_FlightMouse(const FInputActionValue& Value);
	void Input_PitchKeyboard(const FInputActionValue& Value);
	void Input_Roll(const FInputActionValue& Value);
	void Input_Thrust(const FInputActionValue& Value);

	/** 자유 시점(Free Look) 입력 처리 */
	void Input_FreeLookStarted(const FInputActionValue& Value);
	void Input_FreeLookCompleted(const FInputActionValue& Value);

protected:
	/** 현재 기체의 상태 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|State")
	EWingsPawnState CurrentState;

	/** 자유 시점 활성화 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|State")
	bool bIsFreeLooking;

	/** 입력 매핑 컨텍스트 (Enhanced Input) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** 입력 액션 데이터 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Input")
	TObjectPtr<UWingsInputConfigData> InputConfig;

	/** 비행 물리 데이터 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Data")
	TObjectPtr<UWingsFlightData> FlightData;

	/** 조작이 없을 때 자동으로 수평을 맞추는 기능 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Flight")
	bool bEnableAutoLeveling;

	/** 현재 기체가 유지하고 있는 추진력 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Stats|Flight")
	float CurrentThrust;

	/** 카메라 지연(Lag) 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Camera")
	float CameraLagSpeed;

	/** 카메라 회전 지연(Rotation Lag) 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Camera")
	float CameraRotationLagSpeed;

	/** 최소 시야각 (정지/저속 시) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Camera")
	float MinFOV;

	/** 최대 시야각 (최고 속도 시) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Camera")
	float MaxFOV;

	/** 최소 카메라 거리 (정지/저속 시) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Camera")
	float MinArmLength;

	/** 최대 카메라 거리 (최고 속도 시) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Camera")
	float MaxArmLength;

	/** 카메라 효과가 최대로 적용되는 속도 임계값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Camera")
	float DynamicCameraSpeedThreshold;

	/** 자유 시점 회전 감도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Camera")
	float FreeLookSensitivity;

	/** 최대 연료량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wings|Stats|Fuel")
	float MaxFuel;

	/** 현재 남은 연료량 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Stats|Fuel")
	float CurrentFuel;

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

public:
	/** 컴포넌트 게터 */
	UStaticMeshComponent* GetMeshComponent() const { return MeshComponent.Get(); }
	USpringArmComponent* GetSpringArmComponent() const { return SpringArmComponent.Get(); }
	UCameraComponent* GetCameraComponent() const { return CameraComponent.Get(); }
	UInputMappingContext* GetDefaultMappingContext() const { return DefaultMappingContext.Get(); }

	/** 연료 관련 게터 */
	UFUNCTION(BlueprintPure, Category = "Wings|Stats|Fuel")
	float GetCurrentFuel() const { return CurrentFuel; }

	UFUNCTION(BlueprintPure, Category = "Wings|Stats|Fuel")
	float GetFuelPercentage() const { return (MaxFuel > 0.0f) ? (CurrentFuel / MaxFuel) : 0.0f; }
};
