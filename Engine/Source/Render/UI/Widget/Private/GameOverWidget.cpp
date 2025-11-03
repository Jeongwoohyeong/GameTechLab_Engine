#include "pch.h"
#include "Render/UI/Widget/Public/GameOverWidget.h"
#include "ImGui/imgui.h"

IMPLEMENT_CLASS(UGameOverWidget, UWidget)

void UGameOverWidget::Initialize()
{
	FinalScore = 0;
	bRestartButtonClicked = false;
	bQuitButtonClicked = false;
}

void UGameOverWidget::Update()
{
}

void UGameOverWidget::RenderWidget()
{
	// 뷰포트 중앙 계산
	float CenterX = ViewportLeft + ViewportWidth * 0.5f;
	float CenterY = ViewportTop + ViewportHeight * 0.5f;

	// 메뉴 크기
	const float MenuWidth = 400.0f;
	const float MenuHeight = 350.0f;
	const float ButtonWidth = 200.0f;
	const float ButtonHeight = 50.0f;

	// 중앙 위치 설정
	ImGui::SetNextWindowPos(ImVec2(CenterX - MenuWidth * 0.5f, CenterY - MenuHeight * 0.5f));
	ImGui::SetNextWindowSize(ImVec2(MenuWidth, MenuHeight));

	// 윈도우 플래그
	ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoTitleBar |
	                               ImGuiWindowFlags_NoResize |
	                               ImGuiWindowFlags_NoMove |
	                               ImGuiWindowFlags_NoCollapse;

	ImGui::Begin("GameOverMenu", nullptr, WindowFlags);

	// "GAME OVER" 제목
	ImGui::SetCursorPosX((MenuWidth - ImGui::CalcTextSize("GAME OVER").x) * 0.5f);
	ImGui::SetCursorPosY(40.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
	ImGui::Text("GAME OVER");
	ImGui::PopStyleColor();

	// 최종 점수 표시
	char ScoreText[64];
	sprintf_s(ScoreText, "Final Score: %d", FinalScore);
	ImGui::SetCursorPosX((MenuWidth - ImGui::CalcTextSize(ScoreText).x) * 0.5f);
	ImGui::SetCursorPosY(90.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
	ImGui::Text("%s", ScoreText);
	ImGui::PopStyleColor();

	// 재시작 버튼
	ImGui::SetCursorPosX((MenuWidth - ButtonWidth) * 0.5f);
	ImGui::SetCursorPosY(150.0f);

	if (ImGui::Button("Restart", ImVec2(ButtonWidth, ButtonHeight)))
	{
		bRestartButtonClicked = true;
		UE_LOG("[GameOver] Restart button clicked");
	}

	// 종료 버튼
	ImGui::SetCursorPosX((MenuWidth - ButtonWidth) * 0.5f);
	ImGui::SetCursorPosY(220.0f);

	if (ImGui::Button("Quit to Menu", ImVec2(ButtonWidth, ButtonHeight)))
	{
		bQuitButtonClicked = true;
		UE_LOG("[GameOver] Quit button clicked");
	}

	ImGui::End();
}

void UGameOverWidget::ResetButtonStates()
{
	bRestartButtonClicked = false;
	bQuitButtonClicked = false;
}