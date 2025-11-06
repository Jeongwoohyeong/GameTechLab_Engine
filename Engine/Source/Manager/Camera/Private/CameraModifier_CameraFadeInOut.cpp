#include "pch.h"
#include "Manager/Camera/Public/CameraModifier_CameraFadeInOut.h"
#include "Manager/Camera/Public/PlayerCameraManager.h"


namespace
{
    constexpr float MinimuFadeDuration = 0.01f;
}

IMPLEMENT_CLASS(UCameraModifier_CameraFadeInOut, UCameraModifier)

UCameraModifier_CameraFadeInOut::UCameraModifier_CameraFadeInOut()
        : FadeColor(0.0f, 0.0f, 0.0f, 1.0f)
        , FadeOutDuration(0.5f)
        , FadeInDuration(0.5f)
        , HoldDuration(0.0f)
        , StageTimer(0.0f)
        , bIsFading(false)
        , CurrentStage(EFadeStage::None)
{
    // 페이드 효과는 다른 위치/회전 Modifier보다 뒤에서 실행되도록 높은 우선순위를 부여
    Priority = 240;
}

UCameraModifier_CameraFadeInOut::~UCameraModifier_CameraFadeInOut()
{
    
}

void UCameraModifier_CameraFadeInOut::StartFade(float InFadeOutDuration, float InFadeInDuration,
    const FVector4& InFadeColor, float InHoldDuration)
{
    if (!CameraOwner)
    {
        return;
    }

    FadeOutDuration = (InFadeOutDuration < MinimuFadeDuration) ? MinimuFadeDuration : InFadeOutDuration;
    FadeInDuration = (InFadeInDuration < MinimuFadeDuration) ? MinimuFadeDuration : InFadeInDuration;
    HoldDuration = (InHoldDuration < 0.0f) ? 0.0f : InHoldDuration;
    FadeColor = InFadeColor;
    StageTimer = 0.0f;
    bIsFading = true;
    CurrentStage = EFadeStage::FadeOut;

    BeginFadeOut();
    EnableModifierInstant();
}

void UCameraModifier_CameraFadeInOut::StopFade(bool bImmediate)
{
    if (!CameraOwner)
    {
        bIsFading = false;
        CurrentStage = EFadeStage::None;
        return;
    }

    if (bImmediate)
    {
        CameraOwner->StopCameraFade();
    }
    
    bIsFading = false;
    CurrentStage = EFadeStage::None;
    StageTimer = 0.0f;

    DisableModifierInstant();
}

void UCameraModifier_CameraFadeInOut::UpdateModifier(float DeltaTime)
{
    UCameraModifier::UpdateModifier(DeltaTime);

    if (!bIsFading || !CameraOwner)
    {
        return;
    }

    StageTimer += DeltaTime;

    switch (CurrentStage)
    {
    case EFadeStage::FadeOut:
        if (StageTimer >= FadeOutDuration)
        {
            if (HoldDuration > 0.0f)
            {
                CurrentStage = EFadeStage::Hold;
                StageTimer = 0.0f;
            }
            else
            {
                CurrentStage = EFadeStage::FadeIn;
                StageTimer = 0.0f;
                BeginFadeIn();
            }
        }
        break;

    case EFadeStage::Hold:
        if (StageTimer >= HoldDuration)
        {
            CurrentStage = EFadeStage::FadeIn;
            StageTimer = 0.0f;
            BeginFadeIn();
        }
        break;

    case EFadeStage::FadeIn:
        if (StageTimer >= FadeInDuration)
        {
            FinishFade();
        }
        break;

    default:
        break;
    }
}

bool UCameraModifier_CameraFadeInOut::ModifyCamera(float DeltaTime, FVector& CameraLocation, FRotator& CameraRotation)
{
    return false;
}

void UCameraModifier_CameraFadeInOut::SetFadeOutDuration(float InDuration)
{
    FadeOutDuration = InDuration;
}

void UCameraModifier_CameraFadeInOut::SetFadeInDuration(float InDuration)
{
    FadeInDuration = InDuration;
}

void UCameraModifier_CameraFadeInOut::SetHoldDuration(float InDuration)
{
    HoldDuration = InDuration;
}

void UCameraModifier_CameraFadeInOut::SetFadeColor(const FVector4& InColor)
{
    FadeColor = InColor;
}

void UCameraModifier_CameraFadeInOut::BeginFadeOut()
{
    StageTimer = 0.0f;
    if (CameraOwner)
    {
        CameraOwner->StartCameraFade(FadeOutDuration, FadeColor, true, true);
    }
}

void UCameraModifier_CameraFadeInOut::BeginFadeIn()
{
    StageTimer = 0.0f;
    if (CameraOwner)
    {
        CameraOwner->StartCameraFade(FadeInDuration, FadeColor, false, false);
    }
}

void UCameraModifier_CameraFadeInOut::FinishFade()
{
    if (CameraOwner)
    {
        CameraOwner->StopCameraFade();
    }
    bIsFading = false;
    CurrentStage = EFadeStage::None;
    StageTimer = 0.0f;
    DisableModifier();
}
