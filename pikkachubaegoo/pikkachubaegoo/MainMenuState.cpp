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
	UObjectFactory::GetInstance()->CreateBGS();
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

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
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
	ImGui::PopStyleColor();

	// 스타일 원상복귀
	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(2);

	RenderModernControlGuide();

	ImGui::End();
}

// ✅ minWidth 인자를 받고, std::max를 사용해 너비를 보정합니다.
void MainMenuState::DrawKey(const char* key, ImVec2 alignment, float fixedWidth)
{
	ImVec2 cursorPos = ImGui::GetCursorScreenPos();
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec2 textSize = ImGui::CalcTextSize(key);
	ImVec2 padding = ImVec2(style.FramePadding.x * 1.5f, style.FramePadding.y);
	ImVec2 keySize = ImVec2(textSize.x + padding.x * 2, textSize.y + padding.y * 2);

	// ✅ [수정] 이 부분이 빠져있었습니다!
	// fixedWidth가 0보다 크면, 키의 너비를 해당 값으로 강제 고정합니다.
	if (fixedWidth > 0.0f)
	{
		keySize.x = fixedWidth;
	}

	ImRect keyRect(cursorPos, ImVec2(cursorPos.x + keySize.x, cursorPos.y + keySize.y));
	drawList->AddRectFilled(keyRect.Min, keyRect.Max, ImColor(50, 50, 50, 255), 4.0f);
	drawList->AddRect(keyRect.Min, keyRect.Max, ImColor(80, 80, 80, 255), 4.0f);

	ImVec2 textPos = ImVec2(
		keyRect.Min.x + (keyRect.GetWidth() - textSize.x) * alignment.x,
		keyRect.Min.y + (keyRect.GetHeight() - textSize.y) * alignment.y
	);
	drawList->AddText(textPos, ImColor(240, 240, 240, 255), key);

	ImGui::Dummy(keySize);
}

// MainMenuState.cpp

// ✅ 두 개로 나뉘었던 DrawKeys 함수를 아래의 하나로 교체합니다.
void MainMenuState::DrawKeys(const std::vector<std::string>& keys, float fixedWidth)
{
	ImGui::BeginGroup();
	for (size_t i = 0; i < keys.size(); ++i)
	{
		// 전달받은 fixedWidth 값을 DrawKey 함수로 그대로 넘겨줍니다.
		DrawKey(keys[i].c_str(), ImVec2(0.5f, 0.5f), fixedWidth);

		if (i < keys.size() - 1)
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().ItemSpacing.x / 2);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::Text("/");
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().ItemSpacing.x / 2);
		}
	}
	ImGui::EndGroup();
}
void MainMenuState::RenderModernControlGuide()
{
	ImVec2 windowSize = ImGui::GetWindowSize();
	float childHeight = 220.0f;
	ImGui::SetCursorPosY(windowSize.y - childHeight - 30.0f);

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.7f));
	ImGui::BeginChild("ControlGuide", ImVec2(0, childHeight), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NavFlattened);
	ImGui::PopStyleColor();

	ImGui::SeparatorText(u8"조작법");
	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	if (ImGui::BeginTable("controlsTable", 3, ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn(u8"1P", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn(u8"2P", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn(u8"기능", ImGuiTableColumnFlags_WidthStretch);

		// ✅ 헤더를 수동으로 그려서 색상 적용
		ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
		ImGui::TableNextColumn();
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // 1P 헤더 색상
		ImGui::Text(u8"1P");
		ImGui::PopStyleColor();

		ImGui::TableNextColumn();
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 1.0f, 1.0f)); // 2P 헤더 색상
		ImGui::Text(u8"2P");
		ImGui::PopStyleColor();

		ImGui::TableNextColumn();
		ImGui::Text(u8"");

		// ✅ isLongKey bool 대신 float keyWidth를 직접 받도록 람다 시그니처를 변경
		auto DrawControlRow = [](const std::vector<std::string>& p1_keys, const std::vector<std::string>& p2_keys, const char* action, float keyWidth)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.7f, 0.3f, 1.0f));
				DrawKeys(p1_keys, keyWidth); // ◀ 전달받은 너비 값으로 DrawKeys 호출
				ImGui::PopStyleColor();

				ImGui::TableNextColumn();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
				DrawKeys(p2_keys, keyWidth); // ◀ 전달받은 너비 값으로 DrawKeys 호출
				ImGui::PopStyleColor();

				ImGui::TableNextColumn();

				// ✅ [수정] 불안정한 GetContentRegionAvail() 대신 GetFrameHeight()를 기준으로 사용 (행 높이 문제 해결)
				float y_offset = (ImGui::GetFrameHeight() - ImGui::CalcTextSize(action).y) * 0.5f;
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y_offset);
				ImGui::Text("%s", action);
			};

		// 일반 키들은 고정 너비로 ImGui::GetFrameHeight()를 전달
		DrawControlRow({ u8"W" }, { u8"↑" }, u8"점프", ImGui::GetFrameHeight());
		DrawControlRow({ u8"A", u8"D" }, { u8"←", u8"→" }, u8"이동", ImGui::GetFrameHeight());
		DrawControlRow({ u8"S" }, { u8"↓" }, u8"슬라이딩", ImGui::GetFrameHeight());

		// ✅ 긴 키들은 동적 너비를 위해 0.0f를 전달
		DrawControlRow({ u8"SPACE" }, { u8"ENTER" }, u8"스파이크", 0.0f);

		ImGui::EndTable();
	}

	ImGui::EndChild();
}
void MainMenuState::Exit()
{
	UObjectFactory::GetInstance()->ReleaseAll();
	// 메뉴 상태를 빠져나갈 때 호출됩니다.
	// (예: 메뉴 배경음악 정지, 메뉴용 리소스 해제 등)
	// 지금은 특별한 정리 작업이 필요 없으므로 비워둡니다.
}