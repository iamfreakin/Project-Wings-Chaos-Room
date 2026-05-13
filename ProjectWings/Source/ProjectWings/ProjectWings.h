// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWings, Log, All);

/** 
 * 게임 내 오브젝트의 재질 속성 정의 
 */
UENUM(BlueprintType)
enum class EWingsAttribute : uint8
{
	None    UMETA(DisplayName = "None"),
	Stone   UMETA(DisplayName = "Stone"),
	Wood    UMETA(DisplayName = "Wood"),
	Grass   UMETA(DisplayName = "Grass")
};

