#pragma once

class IGameState
{
public:
	virtual ~IGameState() = default;

	// 상태에 진입할 때 한 번 호출됩니다.
	virtual void Enter() = 0;
	// 매 프레임 호출됩니다.
	virtual void Update(float deltaTime) = 0;
	// 렌더링 시 매 프레임 호출됩니다.
	virtual void Render() = 0;
	// 상태를 빠져나갈 때 한 번 호출됩니다.
	virtual void Exit() = 0;
};