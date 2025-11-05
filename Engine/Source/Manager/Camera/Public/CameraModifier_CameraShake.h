#pragma once
#include "Manager/Camera/Public/CameraModifier.h"

/**
 * @brief Camera Shake Modifier
 * 카메라를 무작위로 흔들어서 폭발, 충격 등의 효과를 표현
 */
class UCameraModifier_CameraShake : public UCameraModifier
{
    DECLARE_CLASS(UCameraModifier_CameraShake, UCameraModifier)

public:
    UCameraModifier_CameraShake();
    virtual ~UCameraModifier_CameraShake() = default;

    /**
     * @brief 카메라 흔들림 시작
     * @param Intensity 흔들림 강도 (위치 오프셋 크기)
     * @param Duration 흔들림 지속 시간 (초)
     */
    void StartShake(float Intensity, float Duration);

    /**
     * @brief 카메라 흔들림 즉시 중단
     */
    void StopShake();

    // UCameraModifier override
    virtual bool ModifyCamera(float DeltaTime, FVector& CameraLocation, FRotator& CameraRotation) override;
    virtual void UpdateModifier(float DeltaTime) override;

public:
    // ========== Bezier Decay Curve Settings ==========

    /** Bezier curve control points for decay curve (default: easeOutQuad) */
    float BezierCP[4] = { 0.250f, 0.460f, 0.450f, 0.940f };

    /** Use Bezier curve for decay instead of linear */
    bool bUseBezierDecay = true;

private:
    /** 현재 흔들림 강도 */
    float ShakeIntensity = 0.0f;

    /** 흔들림 지속 시간 */
    float ShakeDuration = 0.0f;

    /** 경과 시간 */
    float ShakeTimer = 0.0f;

    /** 현재 흔들림 오프셋 */
    FVector ShakeOffset = FVector::Zero();

    /** 흔들림 진행 중인지 여부 */
    bool bIsShaking = false;
};