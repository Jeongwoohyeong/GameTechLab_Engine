#pragma once
#include "Types.h"
#include "Math.h"

#define MAX_KEYS 256

class CInputManager
{
public:
	static CInputManager& GetInstance()
	{
		static CInputManager Instance;
		return Instance;
	}

	CInputManager(const CInputManager&) = delete;
	CInputManager& operator=(const CInputManager&) = delete;

	void SetHWnd(HWND* InHWnd) { hWnd = InHWnd; }

	void Update();

	// ---판독 함수 공통 설명---
	// IsDown: 현재 눌려있는 상태
	// IsPrevDown: 이전 프레임에 눌린 상태
	// IsPressed: 이번 프레임에 막 눌린 상태
	// IsReleased: 이번 프레임에 막 떼어진 상태

	bool IsKeyDown(int32 key);

	bool IsPrevKeyDown(int32 key);

	bool IsKeyPressed(int32 key);

	bool IskeyReleased(int32 key);

	bool IsMouseBtnDown(int32 Btn); // 0: left, 1: right

	bool IsPrevMouseBtnDown(int32 Btn);

	bool IsMouseBtnPressed(int32 Btn);

	POINT GetMouseClientPos();

	int32 GetClientW() const;
	int32 GetClientH() const;

public:
	FVector MousePressPosWorld{0.0f, 0.0f, 0.0f}; // 마우스 클릭 시점의 Ray - 투영 평면 교차점 월드 좌표 (기즈모 드래그용)
	FVector MouseCurrentPosWorld{0.0f, 0.0f, 0.0f}; // 현재의 Ray - 투영 평면 교차점 월드 좌표 (기즈모 드래그용)

private:
	CInputManager() {};
	~CInputManager() {};
	bool CheckMouseBtnValid(int32 Btn);
	void UpdateMousePosAndDelta();
	void UpdateClientSize();
	void UpdateKeyStates();

	HWND* hWnd = nullptr;
	int32 ClientW = 0;
	int32 ClientH = 0;
	bool CurrentKeyStates[MAX_KEYS] = { 0 };
	bool PreviousKeyStates[MAX_KEYS] = { 0 };
	bool CurrentMouseBtnStates[2] = { false, false };
	bool PreviousMouseBtnStates[2] = { false, false };
	POINT CurrentMouseClientPosPoint = { 0, 0 };
	POINT PreviousMouseClientPosPoint = { 0, 0 };

	POINT CurrentMouseDelta = { 0, 0 };
};