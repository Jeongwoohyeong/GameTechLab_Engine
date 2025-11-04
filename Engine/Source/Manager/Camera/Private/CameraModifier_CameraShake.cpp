#include "pch.h"
  #include "Manager/Camera/Public/CameraModifier_CameraShake.h"

  IMPLEMENT_CLASS(UCameraModifier_CameraShake, UCameraModifier)

  UCameraModifier_CameraShake::UCameraModifier_CameraShake()
      : ShakeIntensity(0.0f)
      , ShakeDuration(0.0f)
      , ShakeTimer(0.0f)
      , ShakeOffset(FVector::Zero())
      , bIsShaking(false)
  {
      // 기본 Priority 설정 (낮을수록 먼저 적용)
      Priority = 100;
  }

  void UCameraModifier_CameraShake::StartShake(float Intensity, float Duration)
  {
      ShakeIntensity = Intensity;
      ShakeDuration = Duration;
      ShakeTimer = 0.0f;
      bIsShaking = true;

      // Modifier 활성화
      EnableModifier();
  }

  void UCameraModifier_CameraShake::StopShake()
  {
      bIsShaking = false;
      ShakeTimer = 0.0f;
      ShakeOffset = FVector::Zero();

      // Modifier 비활성화
      DisableModifier();
  }

  void UCameraModifier_CameraShake::UpdateModifier(float DeltaTime)
  {
      // Base class의 Alpha 업데이트 (서서히 FadeIn/Out)
      UCameraModifier::UpdateModifier(DeltaTime);

      if (!bIsShaking)
      {
          ShakeOffset = FVector::Zero();
          return;
      }

      // 타이머 업데이트
      ShakeTimer += DeltaTime;

      if (ShakeTimer >= ShakeDuration)
      {
          // 흔들림 종료
          StopShake();
          return;
      }

      // 시간에 따라 감소하는 강도 계산 (점점 약해짐)
      float ShakeAlpha = 1.0f - (ShakeTimer / ShakeDuration);
      float CurrentIntensity = ShakeIntensity * ShakeAlpha;

      // 무작위 오프셋 생성
      ShakeOffset.X = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * CurrentIntensity;
      ShakeOffset.Y = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * CurrentIntensity;
      ShakeOffset.Z = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * CurrentIntensity;
  }

  bool UCameraModifier_CameraShake::ModifyCamera(float DeltaTime, FVector& CameraLocation, FRotator& CameraRotation)
  {
      if (!bIsShaking)
      {
          return false;
      }

      // Alpha 값으로 효과 강도 조절 (FadeIn/Out)
      CameraLocation += ShakeOffset * Alpha;

      return false;  // false = 다른 Modifier도 계속 적용
  }