#pragma once
#include "Widget.h"

/**
 * @brief 게임 오버 위젯
 * 플레이어 사망 시 표시되는 UI
 * - 최종 점수 표시
 * - 재시작 버튼
 * - 종료 버튼
 */
class UGameOverWidget : public UWidget
{
    DECLARE_CLASS(UGameOverWidget, UWidget)

public:
    void Initialize() override;
    void Update() override;
    void RenderWidget() override;

    UGameOverWidget() = default;
    ~UGameOverWidget() override = default;

    // 최종 점수 설정
    void SetFinalScore(int32 InScore) { FinalScore = InScore; }

    // 버튼 클릭 이벤트
    bool IsRestartButtonClicked() const { return bRestartButtonClicked; }
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
    int32 FinalScore = 0;
    bool bRestartButtonClicked = false;
    bool bQuitButtonClicked = false;

    // 뷰포트 영역
    float ViewportLeft = 0.0f;
    float ViewportTop = 0.0f;
    float ViewportWidth = 1920.0f;
    float ViewportHeight = 1080.0f;
};