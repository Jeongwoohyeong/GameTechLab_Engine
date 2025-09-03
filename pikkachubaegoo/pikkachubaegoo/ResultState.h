#pragma once
#include "IGameState.h"
#include <string>

class ResultState : public IGameState
{
private:
	int finalPlayer1Score;
	int finalPlayer2Score;
	std::string resultMessage; // "Player 1 Wins!" 같은 결과 메시지
	std::string scoreMessage;  // "10 : 7" 같은 최종 점수 메시지

public:
	// 생성자에서 최종 점수를 받습니다.
	ResultState(int p1Score, int p2Score);

	// IGameState 인터페이스 함수들을 오버라이드합니다.
	void Enter() override;
	void Update(float deltaTime) override;
	void Render() override;
	void Exit() override;
};