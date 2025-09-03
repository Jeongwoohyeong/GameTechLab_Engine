#include "ResultState.h"
#include "App.h"
#include "PlayingState.h"   // '재시작'을 위해 포함
#include "MainMenuState.h"  // '메인 메뉴로'를 위해 포함
#include "imgui/imgui.h"

ResultState::ResultState(int p1Score, int p2Score)
	: finalPlayer1Score(p1Score), finalPlayer2Score(p2Score)
{
	// 생성자에서 점수를 기반으로 결과 메시지를 미리 만듭니다.
	if (finalPlayer1Score > finalPlayer2Score)
	{
		resultMessage = "Player 1 Wins!";
	}
	else if (finalPlayer2Score > finalPlayer1Score)
	{
		resultMessage = "Player 2 Wins!";
	}
	else
	{
		resultMessage = "Draw!";
	}

	scoreMessage = std::to_string(finalPlayer1Score) + " : " + std::to_string(finalPlayer2Score);
}

void ResultState::Enter()
{
	// 결과 화면 진입 시 필요한 초기화 로직 (예: 결과 BGM 재생)
}

void ResultState::Update(float deltaTime)
{
	// ImGui가 입력을 처리하므로 비워둡니다.
}

void ResultState::Render()
{
	// 전체 화면을 덮는 결과 창을 만듭니다.
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::Begin("ResultScreen", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

	// 1. 결과 메시지 (승자)
	float originalFontSize = ImGui::GetFont()->Scale;
	ImGui::GetFont()->Scale = 5.0f; // 큰 폰트로
	ImGui::PushFont(ImGui::GetFont());

	ImVec2 textSize = ImGui::CalcTextSize(resultMessage.c_str());
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - textSize.x) * 0.5f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.3f);
	ImGui::Text(resultMessage.c_str());

	ImGui::PopFont();

	// 2. 최종 점수
	ImGui::GetFont()->Scale = 3.0f; // 조금 작은 폰트로
	ImGui::PushFont(ImGui::GetFont());

	textSize = ImGui::CalcTextSize(scoreMessage.c_str());
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - textSize.x) * 0.5f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.4f);
	ImGui::Text(scoreMessage.c_str());

	ImGui::PopFont();
	ImGui::GetFont()->Scale = originalFontSize; // 폰트 스케일 원복

	// 3. 재시작 버튼
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 150) * 0.5f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.6f);
	if (ImGui::Button("Restart", ImVec2(150, 50)))
	{
		UApp::Ins->ChangeState(new PlayingState());
	}

	// 4. 메인 메뉴로 돌아가기 버튼
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 150) * 0.5f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.6f + 60);
	if (ImGui::Button("Main Menu", ImVec2(150, 50)))
	{
		UApp::Ins->ChangeState(new MainMenuState());
	}

	ImGui::End();
}

void ResultState::Exit()
{
	// 결과 화면을 나갈 때 필요한 정리 로직
}