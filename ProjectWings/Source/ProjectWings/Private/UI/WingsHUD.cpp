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

    AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>();

    // 1. 선택 위젯 생성
    if (SelectionWidgetClass)
    {
        SelectionWidget = CreateWidget<UUserWidget>(GetWorld(), SelectionWidgetClass);
    }

    // 2. 메인 HUD 위젯 생성
	if (MainHUDWidgetClass)
	{
		MainHUDWidget = CreateWidget<UUserWidget>(GetWorld(), MainHUDWidgetClass);
	}

    // 3. 초기 UI 상태 설정
    ToggleUI();

	// 4. GameMode 이벤트 구독
	if (GM)
	{
		GM->OnGameStateChanged.AddDynamic(this, &AWingsHUD::HandleGameStateChanged);
        GM->OnSequenceChanged.AddDynamic(this, &AWingsHUD::HandleSequenceChanged);
	}
}

void AWingsHUD::HandleSequenceChanged()
{
    ToggleUI();
}

void AWingsHUD::ToggleUI()
{
    AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>();
    if (!GM) return;

    bool bIsConfirmed = GM->IsSequenceConfirmed();
    APlayerController* PC = GetOwningPlayerController();

    // 기체 선택 단계
    if (!bIsConfirmed)
    {
        if (MainHUDWidget) MainHUDWidget->RemoveFromParent();
        if (SelectionWidget && !SelectionWidget->IsInViewport())
        {
            SelectionWidget->AddToViewport();
        }

        // 마우스 커서 활성화 및 입력 모드 설정
        if (PC)
        {
            FInputModeGameAndUI InputMode;
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = true;
        }
    }
    // 발사/비행 단계
    else
    {
        if (SelectionWidget) SelectionWidget->RemoveFromParent();
        if (MainHUDWidget && !MainHUDWidget->IsInViewport())
        {
            MainHUDWidget->AddToViewport();
        }

        // 마우스 커서 비활성화 및 게임 입력 전용 설정
        if (PC)
        {
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = false;
        }
    }
}

void AWingsHUD::HandleGameStateChanged(bool bIsWin)
{
    // 게임 종료 시 기존 UI 제거
    if (SelectionWidget) SelectionWidget->RemoveFromParent();
    if (MainHUDWidget) MainHUDWidget->RemoveFromParent();
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