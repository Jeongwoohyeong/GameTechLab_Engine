#pragma once

/*
점프, 슬라이딩(스파이크) - 한 번만 입력받기

*/

#define MAX_KEYS 256

class UInput
{
public:
	static UInput* GetInstance();

	// 대입, 복사 명시적으로 삭제
	UInput(const UInput&) = delete;
	UInput& operator=(const UInput&) = delete;

	// 매 프레임마다 키보드 상태 갱신
	void Update();

	// 현재 키 눌림 유지 검사
	bool IsKeyDown(int key);

	// 이전에 눌렸었는 지 검사
	bool IsPrevKeyDown(int key);

	// 키가 처음 눌렸는 지 검사
	bool IsKeyPressed(int key);

	// 키가 떼어졌는 지 검사
	bool IsKeyReleased(int key);

private:
	UInput();
	~UInput();

	bool CurrentKeyStates[MAX_KEYS] = { 0 };
	bool PreviousKeyStates[MAX_KEYS] = { 0 };
};