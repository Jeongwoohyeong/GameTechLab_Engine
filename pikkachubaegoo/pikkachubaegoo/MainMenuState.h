#pragma once
#include "IGameState.h"

class MainMenuState : public IGameState
{
public:
	// IGameState 인터페이스 함수들을 오버라이드합니다.
	void Enter() override;
	void Update(float deltaTime) override;
	void Render() override;
	void Exit() override;
};