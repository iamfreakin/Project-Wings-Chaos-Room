// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WingsHUD.h"
#include "Blueprint/UserWidget.h"
#include "Core/WingsGameMode.h"

AWingsHUD::AWingsHUD()
{
}

void AWingsHUD::BeginPlay()
{
	Super::BeginPlay();

	if (MainHUDWidgetClass)
	{
		MainHUDWidget = CreateWidget<UUserWidget>(GetWorld(), MainHUDWidgetClass);
		if (MainHUDWidget)
		{
			MainHUDWidget->AddToViewport();
		}
	}
	// GameMode의 이벤트 알림판에 내 함수(HandleGameStateChanged)를 등록!
	if (AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>())
	{
		GM->OnGameStateChanged.AddDynamic(this, &AWingsHUD::HandleGameStateChanged);
	}
}

void AWingsHUD::HandleGameStateChanged(bool bIsWin)
{
	TSubclassOf<UUserWidget> SelectedClass = bIsWin ? VictoryWidgetClass : GameOverWidgetClass;

	if (SelectedClass)
	{
		if (UUserWidget* ResultWidget = CreateWidget<UUserWidget>(GetWorld(), SelectedClass))
		{
			// 마우스 커서를 보여주고 게임 입력을 UI로 집중시킴 (실무 필수 처리)
			if (APlayerController* PC = GetOwningPlayerController())
			{
				ResultWidget->AddToViewport();
				
				// [오류 수정] 위젯이 포커스를 받을 수 있도록 설정 (에러 로그 방지)
				ResultWidget->SetIsFocusable(true);

				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(ResultWidget->TakeWidget());
				PC->SetInputMode(InputMode);
				PC->bShowMouseCursor = true;

				// 명시적으로 키보드 포커스 부여
				ResultWidget->SetKeyboardFocus();
			}
		}
	}
}