// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "WingsLauncher.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UArrowComponent;
class UWingsInputConfigData;
class UInputMappingContext;
class AWingsPawnBase;
struct FInputActionValue;

UCLASS()
class PROJECTWINGS_API AWingsLauncher : public APawn
{
	GENERATED_BODY()
	
public:	
	AWingsLauncher();

    /** 게터 */
    UInputMappingContext* GetLauncherMappingContext() const { return LauncherMappingContext.Get(); }

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    /** 조준 및 발사 로직 (Pawn에서 이식될 부분) */
    void Input_Aim(const FInputActionValue& Value);
    void Input_LaunchStarted(const FInputActionValue& Value);
    void Input_LaunchCompleted(const FInputActionValue& Value);

    /** 궤적 예측 */
    void UpdateTrajectory();

protected:
	/** 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> LauncherMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArrowComponent> LaunchDirectionIndicator;

    /** 입력 설정 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UWingsInputConfigData> InputConfig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> LauncherMappingContext;

    /** 발사 성능 (나중에 강화 시스템으로 확장 가능) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launcher | Stats")
    float MaxLaunchForce = 10000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launcher | Stats")
    float ChargeSpeed = 0.2f;

    /** 상태 변수 */
    float CurrentLaunchPower = 0.f;
    bool bIsCharging = false;
    FRotator AimRotation = FRotator::ZeroRotator;

    /** 궤적 설정 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launcher | Trajectory")
    float TrajectoryMaxTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launcher | Trajectory")
    float TrajectoryFrequency = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launcher | Trajectory")
    float TrajectoryRadius = 5.0f;

    /** 발사할 기체 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Launcher | Projectile")
    TSubclassOf<AWingsPawnBase> ProjectileClass;
};
