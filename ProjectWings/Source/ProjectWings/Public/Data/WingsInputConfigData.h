// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WingsInputConfigData.generated.h"

class UInputAction;

/**
 * 입력을 위한 InputAction들을 그룹화하여 관리하는 데이터 에셋입니다.
 */
UCLASS()
class PROJECTWINGS_API UWingsInputConfigData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** [Ready] 조준 시 사용하는 액션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Input|Ready")
	TObjectPtr<UInputAction> IA_Aim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Input|Ready")
	TObjectPtr<UInputAction> IA_Launch;

	/** [Flying] 비행 시 사용하는 액션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Input|Flying")
	TObjectPtr<UInputAction> IA_Pitch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Input|Flying")
	TObjectPtr<UInputAction> IA_Roll;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Input|Flying")
	TObjectPtr<UInputAction> IA_Thrust;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Input|Flying")
	TObjectPtr<UInputAction> IA_FreeLook;

	/** [Global/Retry] 재시도시 사용하는 액션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wings|Input")
	TObjectPtr<UInputAction> IA_Retry;
	
};
