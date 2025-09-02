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

	void Update();

	bool IsKeyDown(int key);
	bool IsPrevKeyDown(int key);
	bool IsKeyPressed(int key);
	bool IsKeyReleased(int key);

private:
	UInput();
	~UInput();

	bool CurrentKeyStates[MAX_KEYS] = { 0 };
	bool PreviousKeyStates[MAX_KEYS] = { 0 };
};