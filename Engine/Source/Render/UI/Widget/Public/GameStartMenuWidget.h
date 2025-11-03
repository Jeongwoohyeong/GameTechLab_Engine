#pragma once
#include "Widget.h"

/**
 * @brief 게임 시작 메뉴 위젯
 * PIE 모드 진입 시 표시되는 메인 메뉴
 * - 게임 시작 버튼
 * - 종료 버튼
 */
class UGameStartMenuWidget : public UWidget
{
    DECLARE_CLASS(UGameStartMenuWidget, UWidget)

public:
    void Initialize() override;
    void Update() override;
    void RenderWidget() override;

    UGameStartMenuWidget() = default;
    ~UGameStartMenuWidget() override = default;

    // 버튼 클릭 이벤트
    bool IsStartButtonClicked() const { return bStartButtonClicked; }
    bool IsQuitButtonClicked() const { return bQuitButtonClicked; }
    void ResetButtonStates();

    // 뷰포트 영역 설정
    void SetViewportRect(float InLeft, float InTop, float InWidth, float InHeight)
    {
        ViewportLeft = InLeft;
        ViewportTop = InTop;
        ViewportWidth = InWidth;
        ViewportHeight = InHeight;
    }

private:
    bool bStartButtonClicked = false;
    bool bQuitButtonClicked = false;

    // 뷰포트 영역
    float ViewportLeft = 0.0f;
    float ViewportTop = 0.0f;
    float ViewportWidth = 1920.0f;
    float ViewportHeight = 1080.0f;
};