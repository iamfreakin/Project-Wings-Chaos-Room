// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ProjectWings/ProjectWings.h"
#include "WingsPawnBase.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UNiagaraComponent;
class UFieldSystemComponent;
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

	/** 기체의 재질 속성 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Stats")
	EWingsAttribute Attribute = EWingsAttribute::None;

	/** 자유 시점 활성화 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|State")
	bool bIsFreeLooking;

	/** 사망 카메라 초기 자동 보정 완료 여부 */
	bool bIsDeathCamInitialized;

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

	/** 파괴 충격파를 발산하는 필드 시스템 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wings|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFieldSystemComponent> FieldSystemComponent;

protected:
	/** 충돌 시 파괴 필드를 생성하는 함수 */
	UFUNCTION()
	void OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** 충돌 지점에 물리적 충격파(Strain/Force)를 발산합니다. */
	void SpawnDestructionField(FVector ContactLocation, FVector HitNormal, float DamageMultiplier = 1.0f);

public:
	/** 컴포넌트 게터 */
	UStaticMeshComponent* GetMeshComponent() const { return MeshComponent.Get(); }
	USpringArmComponent* GetSpringArmComponent() const { return SpringArmComponent.Get(); }
	UCameraComponent* GetCameraComponent() const { return CameraComponent.Get(); }
	UInputMappingContext* GetDefaultMappingContext() const;

	/** 물리 관련 게터 */
	UFUNCTION(BlueprintPure, Category = "Wings|Stats|Physics")
	float GetPawnMass() const;

	/** [DEPRECATED] 연료 관련 게터 (항상 1.0 반환) */
	UFUNCTION(BlueprintPure, Category = "Wings|Stats|Fuel (Deprecated)")
	float GetFuelPercentage() const { return 1.0f; }
};
