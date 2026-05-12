// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WingsUserWidget.h"
#include "Pawn/WingsPawnBase.h"
#include "Launcher/WingsLauncher.h"
#include "Kismet/GameplayStatics.h"

AWingsPawnBase* UWingsUserWidget::GetOwningWingsPawn() const
{
    return Cast<AWingsPawnBase>(GetOwningPlayerPawn());
}

AWingsLauncher* UWingsUserWidget::GetActiveLauncher() const
{
    // 보통 월드에 발사대는 하나이므로 GameplayStatics를 통해 찾습니다.
    return Cast<AWingsLauncher>(UGameplayStatics::GetActorOfClass(GetWorld(), AWingsLauncher::StaticClass()));
}

float UWingsUserWidget::GetCurrentSpeedKmh() const
{
    if (AWingsPawnBase* Pawn = GetOwningWingsPawn())
    {
        // Unreal Unit (cm/s) -> Km/h 변환 (3.6 곱하기)
        return Pawn->GetVelocity().Size() * 0.036f;
    }
    return 0.0f;
}

float UWingsUserWidget::GetFuelPercent() const
{
    if (AWingsPawnBase* Pawn = GetOwningWingsPawn())
    {
        return Pawn->GetFuelPercentage();
    }
    return 0.0f;
}

float UWingsUserWidget::GetLaunchPowerPercent() const
{
    if (AWingsLauncher* Launcher = GetActiveLauncher())
    {
        return Launcher->GetCurrentLaunchPower();
    }
    return 0.0f;
}

FText UWingsUserWidget::GetSpeedText() const
{
    int32 SpeedKmh = FMath::RoundToInt(GetCurrentSpeedKmh());
    return FText::Format(NSLOCTEXT("WingsUI", "SpeedFormat", "{0} km/h"), FText::AsNumber(SpeedKmh));
}

FSlateColor UWingsUserWidget::GetFuelColor() const
{
    float Percent = GetFuelPercent();
    
    if (Percent > 0.8f) return FSlateColor(FLinearColor::Green);
    if (Percent > 0.2f) return FSlateColor(FLinearColor::Yellow);
    
    return FSlateColor(FLinearColor::Red);
}

ESlateVisibility UWingsUserWidget::GetLaunchPowerVisibility() const
{
    if (AWingsLauncher* Launcher = GetActiveLauncher())
    {
        return Launcher->IsCharging() ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
    }
    return ESlateVisibility::Hidden;
}

ESlateVisibility UWingsUserWidget::GetFlightUIVisibility() const
{
    // 현재 조종 중인 Pawn이 기체(WingsPawnBase)일 때만 표시
    return Cast<AWingsPawnBase>(GetOwningPlayerPawn()) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
}

ESlateVisibility UWingsUserWidget::GetLauncherUIVisibility() const
{
    // 현재 조종 중인 Pawn이 발사대(WingsLauncher)일 때만 표시
    return Cast<AWingsLauncher>(GetOwningPlayerPawn()) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
}
