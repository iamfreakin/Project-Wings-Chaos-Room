// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "WingsPawnBase.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UNiagaraComponent;

/**
 * 프로젝트의 모든 비행 기체의 베이스가 되는 Pawn 클래스입니다.
 * 물리 기반 이동을 위해 APawn을 상속받습니다.
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

public:
	/** 컴포넌트 게터 */
	UStaticMeshComponent* GetMeshComponent() const { return MeshComponent.Get(); }
	USpringArmComponent* GetSpringArmComponent() const { return SpringArmComponent.Get(); }
	UCameraComponent* GetCameraComponent() const { return CameraComponent.Get(); }
};
