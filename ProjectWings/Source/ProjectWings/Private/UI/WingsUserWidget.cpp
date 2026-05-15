// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WingsUserWidget.h"
#include "Pawn/WingsPawnBase.h"
#include "Launcher/WingsLauncher.h"
#include "Core/WingsGameMode.h"
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

FText UWingsUserWidget::GetRemainingSpawnsText() const
{
    if (AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>())
    {
        int32 Count = GM->GetRemainingSpawns();
        return FText::Format(NSLOCTEXT("WingsUI", "RemainingSpawnsFormat", "남은 기체: {0}"), FText::AsNumber(Count));
    }
    return FText::GetEmpty();
}

ESlateVisibility UWingsUserWidget::GetSelectionUIVisibility() const
{
    if (AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>())
    {
        // 게임 오버가 아니고, 시퀀스가 확정되지 않았을 때만 표시
        if (!GM->IsGameOver() && !GM->IsSequenceConfirmed())
        {
            return ESlateVisibility::Visible;
        }
    }
    return ESlateVisibility::Collapsed;
}

TArray<EWingsAttribute> UWingsUserWidget::GetSelectedSequence() const
{
    if (AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>())
    {
        return GM->GetSelectedSequence();
    }
    return TArray<EWingsAttribute>();
}

bool UWingsUserWidget::IsWaitingForRetry() const
{
    if (AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>())
    {
        return GM->IsWaitingForRetry();
    }
    return false;
}

ESlateVisibility UWingsUserWidget::GetRetryUIVisibility() const
{
    // 1. 재시도 대기 상태여야 함
    // 2. 동시에 현재 조종 중인 대상이 발사대(Launcher)가 아니어야 함 (이미 발사대에 있으면 안내할 필요 없음)
    bool bShouldShow = IsWaitingForRetry() && (Cast<AWingsLauncher>(GetOwningPlayerPawn()) == nullptr);
    
    return bShouldShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
}

int32 UWingsUserWidget::GetRemainingTargets() const
{
    if (AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>())
    {
        return GM->GetRemainingTargets();
    }
    return 0;
}

int32 UWingsUserWidget::GetTotalTargets() const
{
    if (AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>())
    {
        return GM->GetTotalTargets();
    }
    return 0;
}

FText UWingsUserWidget::GetTargetProgressText() const
{
    if (AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>())
    {
        return FText::Format(
            NSLOCTEXT("WingsUI", "TargetProgressFormat", "목표 파괴: {0} / {1}"), 
            FText::AsNumber(GM->GetTotalTargets() - GM->GetRemainingTargets()),
            FText::AsNumber(GM->GetTotalTargets())
        );
    }
    return FText::GetEmpty();
}

ESlateVisibility UWingsUserWidget::GetWinUIVisibility() const
{
    if (AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>())
    {
        return GM->IsGameWon() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
    }
    return ESlateVisibility::Collapsed;
}

ESlateVisibility UWingsUserWidget::GetLossUIVisibility() const
{
    if (AWingsGameMode* GM = GetWorld()->GetAuthGameMode<AWingsGameMode>())
    {
        return GM->IsGameLost() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
    }
    return ESlateVisibility::Collapsed;
}

void UWingsUserWidget::RestartLevel()
{
    FName CurrentLevelName = *GetWorld()->GetMapName();
    // 맵 이름 앞에 붙는 prefix 제거 (보통 "/Game/Maps/TestMap" 식이라 실제 이름만 추출 필요할 수 있음)
    UGameplayStatics::OpenLevel(GetWorld(), FName(*GetWorld()->GetName()));
}

void UWingsUserWidget::QuitGame()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
    }
}

void UWingsUserWidget::LoadNextLevel(FName LevelName)
{
    if (!LevelName.IsNone())
    {
        UGameplayStatics::OpenLevel(GetWorld(), LevelName);
    }
}
