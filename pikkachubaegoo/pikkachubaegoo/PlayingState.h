#pragma once
#include "IGameState.h"
#include <vector>
#include "Object.h" // (가정) UObject 클래스

class PlayingState : public IGameState
{
private:
	// PlayingState 내부에서만 사용하는 세부 게임 상태
	enum class EGameplayState
	{
		Ready,
		InProgress,
		RoundOver
	};

	EGameplayState gameplayState;
	float stateTimer = 0.0f;

	std::vector<UObject*> gameObjects;

public:
	void Enter() override;
	void Update(float deltaTime) override;
	void Render() override;
	void Exit() override;
};