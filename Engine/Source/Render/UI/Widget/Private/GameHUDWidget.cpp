#include "pch.h"
#include "Render/UI/Widget/Public/GameHUDWidget.h"
#include "ImGui/imgui.h"

IMPLEMENT_CLASS(UGameHUDWidget, UWidget)

void UGameHUDWidget::Initialize()
{
	Score = 0;
	Ammo = 100;
	Health = 100.0f;
	MaxHealth = 100.0f;
}

void UGameHUDWidget::Update()
{
}

void UGameHUDWidget::RenderWidget()
{
	// HUD 윈도우 설정 (배경 없음, 타이틀바 없음)
	ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoTitleBar |
	                               ImGuiWindowFlags_NoResize |
	                               ImGuiWindowFlags_NoMove |
	                               ImGuiWindowFlags_NoCollapse |
	                               ImGuiWindowFlags_NoBackground |
	                               ImGuiWindowFlags_NoScrollbar;

	// 뷰포트 툴바 높이 (ViewportControlWidget)
	constexpr float ViewportToolbarHeight = 32.0f;

	// HUD 윈도우 위치 (뷰포트 툴바 아래)
	ImGui::SetNextWindowPos(ImVec2(ViewportLeft, ViewportTop + ViewportToolbarHeight));
	ImGui::SetNextWindowSize(ImVec2(ViewportWidth, 150.0f));

	ImGui::Begin("GameHUD", nullptr, WindowFlags);

	// 왼쪽 상단: 점수
	ImGui::SetCursorPos(ImVec2(20.0f, 20.0f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
	ImGui::Text("Score: %d", Score);
	ImGui::PopStyleColor();

	// 왼쪽 상단 아래: 총알
	ImGui::SetCursorPos(ImVec2(20.0f, 50.0f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 1.0f, 1.0f));
	ImGui::Text("Ammo: %d", Ammo);
	ImGui::PopStyleColor();

	// 오른쪽 상단: 체력바
	ImGui::SetCursorPos(ImVec2(ViewportWidth - 220.0f, 20.0f));
	ImGui::Text("Health:");

	ImGui::SetCursorPos(ImVec2(ViewportWidth - 220.0f, 50.0f));

	// 체력 비율 계산
	float HealthRatio = Health / MaxHealth;

	// 체력에 따라 색상 변경 (높음: 초록, 중간: 노랑, 낮음: 빨강)
	ImVec4 HealthColor;
	if (HealthRatio > 0.5f)
		HealthColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // 초록
	else if (HealthRatio > 0.25f)
		HealthColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // 노랑
	else
		HealthColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // 빨강

	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, HealthColor);
	ImGui::ProgressBar(HealthRatio, ImVec2(200.0f, 30.0f));
	ImGui::PopStyleColor();

	// 체력 수치 표시
	ImGui::SetCursorPos(ImVec2(ViewportWidth - 220.0f, 85.0f));
	ImGui::PushStyleColor(ImGuiCol_Text, HealthColor);
	ImGui::Text("%.0f / %.0f", Health, MaxHealth);
	ImGui::PopStyleColor();

	ImGui::End();
}