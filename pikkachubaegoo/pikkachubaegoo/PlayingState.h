#pragma once
#include "IGameState.h"
#include <vector>
#include "Object.h" // UObject 클래스

class PlayingState : public IGameState
{
private:
	enum class EGameplayState
	{
		Ready,
		InProgress,
		RoundOver,
		End
	};

	EGameplayState gameplayState;
	float stateTimer = 0.0f;

	// --- 추가된 변수 ---
	int player1Score = 0;
	int player2Score = 0;
	UObject* ball = nullptr; // 공 오브젝트를 직접 참조하기 위한 포인터
	UObject* player1 = nullptr;
	UObject* player2 = nullptr;
public:
	void Enter() override;
	void Update(float deltaTime) override;
	void Render() override;
	void Exit() override;

private:
	// 라운드를 초기 상태로 리셋하는 함수
	void ResetRound();
};