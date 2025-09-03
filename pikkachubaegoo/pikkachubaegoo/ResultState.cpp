#include "ResultState.h"
#include "App.h"
#include "PlayingState.h"   // '재시작'을 위해 포함
#include "MainMenuState.h"  // '메인 메뉴로'를 위해 포함
#include "imgui/imgui.h"
#include "ObjectFactory.h"
#include "Background.h"

ResultState::ResultState(int p1Score, int p2Score)
	: finalPlayer1Score(p1Score), finalPlayer2Score(p2Score)
{
	// 생성자에서 점수를 기반으로 결과 메시지를 미리 만듭니다.
	if (finalPlayer1Score > finalPlayer2Score)
	{
		resultMessage = u8"플레이어1 승리!";
	}
	else if (finalPlayer2Score > finalPlayer1Score)
	{
		resultMessage = u8"플레이어2 승리!";
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
	UObjectFactory::GetInstance()->CreateBG(UBackground::BGGroundSpriteAtlasKey, UMeshRenderer::BGGroundOrder);
}

void ResultState::Update(float deltaTime)
{
	// ImGui가 입력을 처리하므로 비워둡니다.
}

void ResultState::Render()
{
	UObjectFactory::GetInstance()->Render();

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::Begin("ResultScreen", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

	// 승자 색상
	ImVec4 winColor;
	if (finalPlayer1Score > finalPlayer2Score)
		winColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // 빨강
	else if (finalPlayer2Score > finalPlayer1Score)
		winColor = ImVec4(0.0f, 0.5f, 1.0f, 1.0f); // 파랑
	else
		winColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // 노랑

	// --- 결과 메시지 (엄청 크게) ---
	float originalFontSize = ImGui::GetFont()->Scale;
	ImGui::GetFont()->Scale = 5.0f; // 큰 폰트로
	ImGui::PushFont(ImGui::GetFont());

	ImVec2 textSize = ImGui::CalcTextSize(resultMessage.c_str());
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - textSize.x) * 0.5f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.3f);
	ImGui::GetWindowDrawList()->AddText(
		ImVec2((ImGui::GetWindowWidth() - ImGui::CalcTextSize(resultMessage.c_str()).x) * 0.5f, ImGui::GetWindowHeight() * 0.2f),
		ImColor(winColor),
		resultMessage.c_str()
	);

	ImGui::PopFont();
	ImGui::GetFont()->Scale = originalFontSize;

	// --- 점수 (엄청 크게, 메시지 바로 아래) ---
	ImGui::GetFont()->Scale = 3.0f; // 조금 작은 폰트로
	ImGui::PushFont(ImGui::GetFont());

	textSize = ImGui::CalcTextSize(scoreMessage.c_str());
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - textSize.x) * 0.5f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.4f);
	ImGui::Text(scoreMessage.c_str());

	ImGui::PopFont();
	ImGui::GetFont()->Scale = originalFontSize; // 폰트 스케일 원복

	// --- 버튼 스타일 ---
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 10));
	ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 200, 100, 255));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 170, 70, 255));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 140, 50, 255));

	// --- 재시작 버튼 ---
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 200) * 0.5f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.55f);
	if (ImGui::Button(u8"다시하기", ImVec2(200, 60)))
	{
		UApp::Ins->ChangeState(new PlayingState());
	}

	// --- 메인 메뉴 버튼 ---
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 200) * 0.5f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.55f + 80);
	if (ImGui::Button(u8"처음으로", ImVec2(200, 60)))
	{
		UApp::Ins->ChangeState(new MainMenuState());
	}

	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(2);

	// --- 크레딧 ---
	ImGui::GetFont()->Scale = 2.0f; // 조금 작은 폰트로
	ImGui::PushFont(ImGui::GetFont());

	const char* creditsText = u8"개발자 - 국동희, 김진철, 김호민, 정우형";
	textSize = ImGui::CalcTextSize(creditsText);
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - textSize.x) * 0.5f);
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.9f);
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), creditsText);

	ImGui::PopFont();
	ImGui::GetFont()->Scale = originalFontSize; // 폰트 스케일 원복

	ImGui::End();
}


void ResultState::Exit()
{
	// 결과 화면을 나갈 때 필요한 정리 로직
	UObjectFactory::GetInstance()->ReleaseAll();
}