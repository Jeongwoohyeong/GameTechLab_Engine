#include "pch.h"
#include "Render/UI/Widget/Public/GameStartMenuWidget.h"
#include "ImGui/imgui.h"

IMPLEMENT_CLASS(UGameStartMenuWidget, UWidget)

void UGameStartMenuWidget::Initialize()
{
	bStartButtonClicked = false;
	bQuitButtonClicked = false;
}

void UGameStartMenuWidget::Update()
{
}

void UGameStartMenuWidget::RenderWidget()
{
	// 뷰포트 중앙 계산
	float CenterX = ViewportLeft + ViewportWidth * 0.5f;
	float CenterY = ViewportTop + ViewportHeight * 0.5f;

	// 메뉴 크기
	const float MenuWidth = 400.0f;
	const float MenuHeight = 300.0f;
	const float ButtonWidth = 200.0f;
	const float ButtonHeight = 50.0f;

	// 중앙 위치 설정
	ImGui::SetNextWindowPos(ImVec2(CenterX - MenuWidth * 0.5f, CenterY - MenuHeight * 0.5f));
	ImGui::SetNextWindowSize(ImVec2(MenuWidth, MenuHeight));

	// 윈도우 플래그 (타이틀바 없음, 이동/크기조절 불가)
	ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoTitleBar |
	                               ImGuiWindowFlags_NoResize |
	                               ImGuiWindowFlags_NoMove |
	                               ImGuiWindowFlags_NoCollapse;

	ImGui::Begin("GameStartMenu", nullptr, WindowFlags);

	// 제목
	ImGui::SetCursorPosX((MenuWidth - ImGui::CalcTextSize("FLIGHT GAME").x) * 0.5f);
	ImGui::SetCursorPosY(50.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
	ImGui::Text("FLIGHT GAME");
	ImGui::PopStyleColor();

	// 버튼들을 중앙에 배치
	ImGui::SetCursorPosX((MenuWidth - ButtonWidth) * 0.5f);
	ImGui::SetCursorPosY(120.0f);

	// 게임 시작 버튼
	if (ImGui::Button("Start Game", ImVec2(ButtonWidth, ButtonHeight)))
	{
		bStartButtonClicked = true;
		UE_LOG("[GameStartMenu] Start Game button clicked");
	}

	ImGui::SetCursorPosX((MenuWidth - ButtonWidth) * 0.5f);
	ImGui::SetCursorPosY(190.0f);

	// 종료 버튼
	if (ImGui::Button("Quit", ImVec2(ButtonWidth, ButtonHeight)))
	{
		bQuitButtonClicked = true;
		UE_LOG("[GameStartMenu] Quit button clicked");
	}

	ImGui::End();
}

void UGameStartMenuWidget::ResetButtonStates()
{
	bStartButtonClicked = false;
	bQuitButtonClicked = false;
}