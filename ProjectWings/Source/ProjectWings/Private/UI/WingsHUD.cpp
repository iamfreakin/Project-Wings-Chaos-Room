// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WingsHUD.h"
#include "Blueprint/UserWidget.h"

AWingsHUD::AWingsHUD()
{
}

void AWingsHUD::BeginPlay()
{
	Super::BeginPlay();

	// 월드에 위젯 클래스가 설정되어 있다면 생성하여 뷰포트에 추가
	if (MainHUDWidgetClass)
	{
		MainHUDWidget = CreateWidget<UUserWidget>(GetWorld(), MainHUDWidgetClass);
		if (MainHUDWidget)
		{
			MainHUDWidget->AddToViewport();
		}
	}
}
