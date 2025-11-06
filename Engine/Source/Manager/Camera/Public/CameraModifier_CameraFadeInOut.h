#pragma once
#include "Manager/Camera/Public/CameraModifier.h"


class UCameraModifier_CameraFadeInOut : public UCameraModifier
{
    DECLARE_CLASS(UCameraModifier_CameraFadeInOut, UCameraModifier);
public:
public:
    UCameraModifier_CameraFadeInOut();
    virtual ~UCameraModifier_CameraFadeInOut();

    /**
     * @brief 화면을 지정 색상으로 페이드 아웃 후 다시 페이드 인
     * @param InFadeOutDuration 색으로 사라지는 시간(초)
     * @param InFadeInDuration 색에서 복귀하는 시간(초)
     * @param InFadeColor RGBA 색상 (W = 알파)
     * @param InHoldDuration 완전 암전 상태 유지 시간(초)
     */
    void StartFade(
            float InFadeOutDuration,
            float InFadeInDuration,
            const FVector4& InFadeColor = FVector4(0.0f, 0.0f, 0.0f, 1.0f),
            float InHoldDuration = 0.0f
    );

    /**
     * @brief 진행 중인 페이드를 중단
     * @param bImmediate true면 즉시 화면을 밝게 복귀
     */
    void StopFade(bool bImmediate = false);

    bool IsFading() const { return bIsFading; }
    FVector4 GetFadeColor() const { return FadeColor; }
    float GetFadeOutDuration() const { return FadeOutDuration; }
    float GetFadeInDuration() const { return FadeInDuration; }
    float GetHoldDuration() const { return HoldDuration; }

    // UCameraModifier override
    virtual void UpdateModifier(float DeltaTime) override;
    virtual bool ModifyCamera(float DeltaTime, FVector& CameraLocation, FRotator& CameraRotation) override;

private:
    enum class EFadeStage : uint8
    {
        None,
        FadeOut,
        Hold,
        FadeIn
};

    void BeginFadeOut();
    void BeginFadeIn();
    void FinishFade();

    

    FVector4 FadeColor;
    float FadeOutDuration;
    float FadeInDuration;
    float HoldDuration;
    float StageTimer;
    bool bIsFading;
    EFadeStage CurrentStage;
};