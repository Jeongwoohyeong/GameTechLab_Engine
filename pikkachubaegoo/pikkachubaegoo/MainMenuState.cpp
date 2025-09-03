#include "MainMenuState.h"
#include "App.h"                // UApp의 인스턴스(Ins)와 ChangeState()를 사용하기 위함
#include "PlayingState.h"       // PlayingState로 전환하기 위함
#include "ImGui/imgui.h"        // ImGui UI를 사용하기 위함

void MainMenuState::Enter()
{
	// 메뉴 상태에 처음 진입했을 때 필요한 초기화 로직을 넣습니다.
	// (예: 메뉴 배경음악 재생, 메뉴용 리소스 로딩 등)
	// 지금은 특별한 초기화가 필요 없으므로 비워둡니다.
}

void MainMenuState::Update(float deltaTime)
{
	// ImGui는 Render 함수에서 UI 렌더링과 업데이트(입력 처리)를 함께 처리하는
	// 'Immediate Mode' 방식을 사용하므로, 이 Update 함수는 비워둬도 괜찮습니다.
	// 만약 메뉴에 움직이는 배경 같은 것이 있다면 여기서 업데이트 로직을 처리합니다.
}

void MainMenuState::Render()
{
	// ImGui를 사용하여 메뉴 창을 만듭니다.
	// 화면을 꽉 채우는 보이지 않는 창을 만들어 UI 요소들을 중앙에 배치합니다.
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::Begin("MainMenu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground);

	// --- UI 요소 배치 ---

	// 게임 제목
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("MY GAME").x) * 0.5f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.3f);
	ImGui::Text("MY GAME");

	// 게임 시작 버튼
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 150) * 0.5f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.5f);
	if (ImGui::Button("Game Start", ImVec2(150, 50)))
	{
		// 버튼이 클릭되면 UApp에 PlayingState로 상태를 변경해달라고 요청합니다.
		UApp::Ins->ChangeState(new PlayingState());
	}

	// 게임 종료 버튼
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 150) * 0.5f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.5f + 60); // 시작 버튼 아래에 배치
	if (ImGui::Button("Exit Game", ImVec2(150, 50)))
	{
		// 윈도우에 종료 메시지를 보내 프로그램을 종료시킵니다.
		PostMessage(UApp::Ins->HWnd, WM_QUIT, 0, 0);
	}

	ImGui::End();
}

void MainMenuState::Exit()
{
	// 메뉴 상태를 빠져나갈 때 호출됩니다.
	// (예: 메뉴 배경음악 정지, 메뉴용 리소스 해제 등)
	// 지금은 특별한 정리 작업이 필요 없으므로 비워둡니다.
}