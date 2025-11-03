#pragma once
#include "Widget.h"

/**
 * @brief 인게임 HUD 위젯
 * 게임 플레이 중 지속적으로 표시되는 UI
 * - 점수 (Score)
 * - 총알 갯수 (Ammo)
 * - 비행기 체력/정보
 */
class UGameHUDWidget : public UWidget
{
    DECLARE_CLASS(UGameHUDWidget, UWidget)

public:
    void Initialize() override;
    void Update() override;
    void RenderWidget() override;

    UGameHUDWidget() = default;
    ~UGameHUDWidget() override = default;

    // HUD 데이터 설정
    void SetScore(int32 InScore) { Score = InScore; }
    void SetAmmo(int32 InAmmo) { Ammo = InAmmo; }
    void SetHealth(float InHealth) { Health = InHealth; }
    void SetMaxHealth(float InMaxHealth) { MaxHealth = InMaxHealth; }

    // HUD 데이터 증가/감소
    void AddScore(int32 Amount) { Score += Amount; }
    void ConsumeAmmo(int32 Amount) { Ammo -= Amount; if (Ammo < 0) Ammo = 0; }
    void TakeDamage(float Amount) {
        Health -= Amount;
        if (Health < 0.0f) Health = 0.0f;
        // 데미지 효과 시작
        DamageEffectTimer = DamageEffectDuration;
    }

    // Getter
    int32 GetScore() const { return Score; }
    int32 GetAmmo() const { return Ammo; }
    float GetHealth() const { return Health; }
    bool IsPlayerDead() const { return Health <= 0.0f; }

    // 뷰포트 영역 설정
    void SetViewportRect(float InLeft, float InTop, float InWidth, float InHeight)
    {
        ViewportLeft = InLeft;
        ViewportTop = InTop;
        ViewportWidth = InWidth;
        ViewportHeight = InHeight;
    }

private:
    int32 Score = 0;
    int32 Ammo = 100;
    float Health = 100.0f;
    float MaxHealth = 100.0f;

    // 뷰포트 영역
    float ViewportLeft = 0.0f;
    float ViewportTop = 0.0f;
    float ViewportWidth = 1920.0f;
    float ViewportHeight = 1080.0f;

    // 데미지 효과
    float DamageEffectTimer = 0.0f;
    const float DamageEffectDuration = 0.5f;  // 0.5초간 효과 지속
};