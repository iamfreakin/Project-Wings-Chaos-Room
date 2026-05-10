// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/WingsGameMode.h"
#include "Core/WingsGameState.h"
#include "Core/WingsPlayerController.h"
#include "Pawn/WingsPawnBase.h"

AWingsGameMode::AWingsGameMode()
{
	// 기본 클래스 설정
	GameStateClass = AWingsGameState::StaticClass();
	PlayerControllerClass = AWingsPlayerController::StaticClass();
	DefaultPawnClass = AWingsPawnBase::StaticClass();
}
