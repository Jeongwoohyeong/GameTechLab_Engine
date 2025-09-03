#pragma once
#include "IGameState.h"
#include "ImGui/imgui.h" // ✅ 이 헤더 파일을 추가해야 합니다.

#include <vector>
#include <string>

class MainMenuState : public IGameState
{
public:
	// IGameState 인터페이스 함수들을 오버라이드합니다.
	void Enter() override;
	void Update(float deltaTime) override;
	void Render() override;
	void Exit() override;
private:
	static void DrawKey(const char* key, ImVec2 alignment = ImVec2(0.5f, 0.5f), float fixedWidth = 0.0f);
	static void DrawKeys(const std::vector<std::string>& keys, float fixedWidth);
	static void RenderModernControlGuide();
};