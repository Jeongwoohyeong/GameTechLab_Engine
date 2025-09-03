#include "MainMenuState.h"
#include "App.h"                // UApp의 인스턴스(Ins)와 ChangeState()를 사용하기 위함
#include "PlayingState.h"       // PlayingState로 전환하기 위함
#include "ImGui/imgui.h"        // ImGui UI를 사용하기 위함
#include "ObjectFactory.h"
#include "Background.h"

void MainMenuState::Enter()
{
	// 메뉴 상태에 처음 진입했을 때 필요한 초기화 로직을 넣습니다.
	// (예: 메뉴 배경음악 재생, 메뉴용 리소스 로딩 등)
	// 지금은 특별한 초기화가 필요 없으므로 비워둡니다.
	UObjectFactory::GetInstance()->CreateBG(UBackground::BGGroundSpriteAtlasKey, UMeshRenderer::BGGroundOrder);
}

void MainMenuState::Update(float deltaTime)
{
	// ImGui는 Render 함수에서 UI 렌더링과 업데이트(입력 처리)를 함께 처리하는
	// 'Immediate Mode' 방식을 사용하므로, 이 Update 함수는 비워둬도 괜찮습니다.
	// 만약 메뉴에 움직이는 배경 같은 것이 있다면 여기서 업데이트 로직을 처리합니다.
}

void MainMenuState::Render()
{
	UObjectFactory::GetInstance()->Render();

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::Begin("MainMenu", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoBackground);



	// 폰트 크기를 일시적으로 키웁니다.
	float originalFontSize = ImGui::GetFont()->Scale;
	ImGui::GetFont()->Scale = 6.0f; // 6배 크기로 설정
	ImGui::PushFont(ImGui::GetFont()); // 변경된 폰트 적용

	// --- 게임 제목 ---
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // 폰트를 크게 적용했다면 여기서 선택
	ImVec2 titleSize = ImGui::CalcTextSize(u8"피카츄 배구");
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - titleSize.x) * 0.5f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.2f);

	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 215, 0, 255)); // 금색 느낌
	ImGui::Text(u8"피카츄 배구");
	ImGui::PopStyleColor();
	ImGui::PopFont();

	// 폰트 크기를 원래대로 복원합니다.
	ImGui::PopFont();
	ImGui::GetFont()->Scale = originalFontSize;

	// --- 버튼 스타일 설정 ---
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 10));
	ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 200, 100, 255));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 170, 70, 255));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 140, 50, 255));

	// --- 게임 시작 버튼 ---
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 200) * 0.5f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.5f);
	if (ImGui::Button(u8"게임 시작", ImVec2(200, 60)))
	{
		UApp::Ins->ChangeState(new PlayingState());
	}

	// --- 게임 종료 버튼 ---
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 200) * 0.5f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.5f + 80);
	if (ImGui::Button(u8"게임 종료", ImVec2(200, 60)))
	{
		PostMessage(UApp::Ins->HWnd, WM_QUIT, 0, 0);
	}

	// 스타일 원상복귀
	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(2);

	ImGui::End();
}

void MainMenuState::Exit()
{
	UObjectFactory::GetInstance()->ReleaseAll();
	// 메뉴 상태를 빠져나갈 때 호출됩니다.
	// (예: 메뉴 배경음악 정지, 메뉴용 리소스 해제 등)
	// 지금은 특별한 정리 작업이 필요 없으므로 비워둡니다.
}