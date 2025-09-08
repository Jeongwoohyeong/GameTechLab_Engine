#include <Windows.h>
#include "CInputManager.h"

void CInputManager::Update()
{
	// 클라이언트 영역 크기 및 마우스 좌표 계산
	POINT CurMousePos = { 0, 0 };
	RECT ClientRect = { 0, 0, 0, 0 };
	GetCursorPos(&CurMousePos); // 현재 마우스 좌표 (화면 좌표)
	ScreenToClient(*hWnd, &CurMousePos); // 클라이언트 좌표로 변환 (윈도우 내부 좌표)
	GetClientRect(*hWnd, &ClientRect); // 클라이언트 영역 크기 얻기
	
	// 클라이언트 영역 크기 저장
	ClientW = ClientRect.right - ClientRect.left;
	ClientH = ClientRect.bottom - ClientRect.top;

	// 이전 프레임 상태 저장
	CopyMemory(PreviousKeyStates, CurrentKeyStates, sizeof(bool) * MAX_KEYS);
	CopyMemory(PreviousMouseBtnStates, CurrentMouseBtnStates, sizeof(bool) * 2);
	PreviousMouseClientPosPoint = CurrentMouseClientPosPoint;

	// 현재 프레임 상태 갱신
	for (int i = 0; i < MAX_KEYS; i++)
	{
		CurrentKeyStates[i] = GetAsyncKeyState(i) & 0x8000; //현재 눌려있는지 여부
	}
	CurrentMouseBtnStates[0] = GetAsyncKeyState(VK_LBUTTON) & 0x8000; //왼쪽 버튼
	CurrentMouseBtnStates[1] = GetAsyncKeyState(VK_RBUTTON) & 0x8000; //오른쪽 버튼
	CurrentMouseClientPosPoint = CurMousePos;
}

bool CInputManager::IsKeyDown(int32 key)
{
	return CurrentKeyStates[key];
}

bool CInputManager::IsPrevKeyDown(int32 key)
{
	return PreviousKeyStates[key];
}

bool CInputManager::IsKeyPressed(int32 key)
{
	return IsKeyDown(key) && !IsPrevKeyDown(key);
}

bool CInputManager::IskeyReleased(int32 key)
{
	return IsPrevKeyDown(key) && !IsKeyDown(key);
}

bool CInputManager::IsMouseBtnDown(int32 Btn) // 0: left, 1: right
{
	return CheckMouseBtnValid(Btn) && CurrentMouseBtnStates[Btn];
}

bool CInputManager::IsPrevMouseBtnDown(int32 Btn)
{
	return CheckMouseBtnValid(Btn) && PreviousMouseBtnStates[Btn];
}

bool CInputManager::IsMouseBtnPressed(int32 Btn)
{
	return CheckMouseBtnValid(Btn) && IsMouseBtnDown(Btn) && !IsPrevMouseBtnDown(Btn);
}

bool CInputManager::CheckMouseBtnValid(int32 Btn)
{
	return (Btn == 0 || Btn == 1);
}

POINT CInputManager::GetMouseClientPos()
{
	return CurrentMouseClientPosPoint;
}

int32 CInputManager::GetClientW() const
{
	return ClientW;
}

int32 CInputManager::GetClientH() const
{
	return ClientH;
}
