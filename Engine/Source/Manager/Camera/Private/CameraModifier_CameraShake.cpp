#include "pch.h"
  #include "Manager/Camera/Public/CameraModifier_CameraShake.h"

  IMPLEMENT_CLASS(UCameraModifier_CameraShake, UCameraModifier)

  namespace
  {
      /**
       * @brief Calculate X coordinate of cubic Bezier curve at parameter t
       */
      float BezierX(float t, const float P[4])
      {
          float oneMinusT = 1.0f - t;
          float oneMinusT2 = oneMinusT * oneMinusT;
          float t2 = t * t;

          // X(t) = 3(1-t)²t * P1.x + 3(1-t)t² * P2.x + t³
          return 3.0f * oneMinusT2 * t * P[0] +
                 3.0f * oneMinusT * t2 * P[2] +
                 t2 * t;
      }

      /**
       * @brief Calculate Y coordinate of cubic Bezier curve at parameter t
       */
      float BezierY(float t, const float P[4])
      {
          float oneMinusT = 1.0f - t;
          float oneMinusT2 = oneMinusT * oneMinusT;
          float t2 = t * t;

          // Y(t) = 3(1-t)²t * P1.y + 3(1-t)t² * P2.y + t³
          return 3.0f * oneMinusT2 * t * P[1] +
                 3.0f * oneMinusT * t2 * P[3] +
                 t2 * t;
      }

      /**
       * @brief Evaluate cubic Bezier curve at time x (X coordinate)
       * Uses Newton-Raphson method to find parameter t for given x
       * @param x Time value [0, 1] (X coordinate)
       * @param P Control points [4]: P1.x, P1.y, P2.x, P2.y (P0=(0,0), P3=(1,1) are fixed)
       * @return Y value at time x
       */
      float EvaluateBezier(float x, const float P[4])
      {
          // Clamp x to [0, 1]
          x = x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x;

          // Edge cases
          if (x <= 0.0f) return 0.0f;
          if (x >= 1.0f) return 1.0f;

          // Newton-Raphson method to find t such that BezierX(t) = x
          float t = x;  // Initial guess
          const int maxIterations = 8;
          const float epsilon = 0.001f;

          for (int i = 0; i < maxIterations; i++)
          {
              float currentX = BezierX(t, P);
              float diff = currentX - x;

              if (fabsf(diff) < epsilon)
                  break;

              // Derivative of X(t)
              float oneMinusT = 1.0f - t;
              float dxdt = 3.0f * oneMinusT * oneMinusT * P[0] +
                           6.0f * oneMinusT * t * (P[2] - P[0]) +
                           3.0f * t * t * (1.0f - P[2]);

              if (fabsf(dxdt) < 0.000001f)
                  break;

              t = t - diff / dxdt;
              t = t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;
          }

          return BezierY(t, P);
      }
  }


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
      float t = ShakeTimer / ShakeDuration;  // [0, 1]
      float ShakeAlpha;

      if (bUseBezierDecay)
      {
          // Use Bezier curve for decay
          float BezierT = EvaluateBezier(t, BezierCP);
          ShakeAlpha = 1.0f - BezierT;  // Invert: start at 1.0, decay to 0.0
      }
      else
      {
          // Linear decay
          ShakeAlpha = 1.0f - t;
      }

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