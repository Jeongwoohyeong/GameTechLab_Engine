#include <Windows.h>
#include "CInputManager.h"
#include "UUIManager.h"
#include "UCamera.h"

void CInputManager::Update()
{
	// 클라이언트 영역 크기 및 마우스 좌표, 버튼 상태 계산
	UpdateMousePosAndDelta();
	UpdateClientSize();
	UpdateKeyStates();

	// 마우스 클릭 이벤트 감지
	if (IsMouseBtnPressed(0)) // 왼쪽 버튼이 막 눌렸을 때
	{
		FDragMouseData clickMouseData = {
			CurrentMouseClientPosPoint.x,
			CurrentMouseClientPosPoint.y,
			ClientW,
			ClientH
		};

		for (int i = 0; i < OnClickCallbacks.size(); i++)
		{
			if (!IsLiveClickCallbacks[i] || OnClickCallbacks[i] == nullptr)
				continue;

			OnClickCallbacks[i](clickMouseData); // 등록된 모든 함수 호출
		}
	}
	if (IsMouseBtnDown(0)) // 드래그할 때 쓸 거
	{
		FDragMouseData dragMouseData = {
			CurrentMouseClientPosPoint.x,
			CurrentMouseClientPosPoint.y,
			ClientW,
			ClientH
		};

		for (int i = 0; i < OnDragCallbacks.size(); i++)
		{
			if (!IsLiveDragCallbacks[i] || OnDragCallbacks[i] == nullptr)
				continue;

			OnDragCallbacks[i](dragMouseData); // 등록된 모든 함수 호출
		}
	}
	// 마우스 떼기 이벤트 감지
	if (IskeyReleased(VK_LBUTTON)) // 왼쪽 버튼이 막 떼어졌을 때
	{
		// UE_LOG("Release");
		for (int i = 0; i < OnReleaseCallbacks.size(); i++)
		{
			if (!IsLiveReleaseCallbacks[i] || OnReleaseCallbacks[i] == nullptr)
				continue;

			OnReleaseCallbacks[i](); // 등록된 모든 함수 호출
		}
	}
}

void CInputManager::UpdateMousePosAndDelta()
{
	POINT CurMousePos = { 0, 0 };

	GetCursorPos(&CurMousePos); // 현재 마우스 좌표 (화면 좌표)
	ScreenToClient(*hWnd, &CurMousePos); // 클라이언트 좌표로 변환 (윈도우 내부 좌표)
	CurrentMouseClientPosPoint = CurMousePos;

	// ΔX, ΔY 계산
	int deltaX = CurrentMouseClientPosPoint.x - PreviousMouseClientPosPoint.x;
	int deltaY = CurrentMouseClientPosPoint.y - PreviousMouseClientPosPoint.y;

	CurrentMouseDelta = { deltaX, deltaY };

	// 이번 프레임 좌표를 다음 프레임을 위해 저장
	PreviousMouseClientPosPoint = CurrentMouseClientPosPoint;
}

void CInputManager::UpdateClientSize()
{
	RECT ClientRect = { 0, 0, 0, 0 };
	GetClientRect(*hWnd, &ClientRect); // 클라이언트 영역 크기 얻기

	// 클라이언트 영역 크기 저장
	ClientW = ClientRect.right - ClientRect.left;
	ClientH = ClientRect.bottom - ClientRect.top;
}

void CInputManager::UpdateKeyStates()
{
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
}

void CInputManager::ReleaseCallbacks()
{
	OnClickCallbacks.clear();
	OnDragCallbacks.clear();
	OnReleaseCallbacks.clear();
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

POINT CInputManager::GetMouseClientPos() { return CurrentMouseClientPosPoint; }
int32 CInputManager::GetClientW() const { return ClientW; }
int32 CInputManager::GetClientH() const { return ClientH; }

int CInputManager::RegisterMouseClickCallback(MouseCallback callback)
{
	OnClickCallbacks.push_back(callback);
	IsLiveClickCallbacks.push_back(true);
	
	int idx = static_cast<int>(OnClickCallbacks.size() - 1);
	return idx;
}

int CInputManager::RegisterMouseDragCallback(MouseCallback callback)
{
	OnDragCallbacks.push_back(callback);
	IsLiveDragCallbacks.push_back(true);

	int idx = static_cast<int>(OnDragCallbacks.size() - 1);
	return idx;
}

int CInputManager::RegisterMouseReleaseCallback(MouseReleaseCallback callback)
{
	OnReleaseCallbacks.push_back(callback);
	IsLiveReleaseCallbacks.push_back(true);

	int idx = static_cast<int>(OnReleaseCallbacks.size() - 1);
	return idx;
}

void CInputManager::RemoveMouseClickCallback(int idx)
{
	if (idx >= OnClickCallbacks.size())
		return;

	if (IsLiveClickCallbacks[idx])
	{
		IsLiveClickCallbacks[idx] = false;
		OnClickCallbacks[idx] = nullptr;
	}
}

void CInputManager::RemoveMouseDragCallback(int idx)
{
	if (idx >= OnDragCallbacks.size())
		return;

	if (IsLiveDragCallbacks[idx])
	{
		IsLiveDragCallbacks[idx] = false;
		OnDragCallbacks[idx] = nullptr;
	}
}

void CInputManager::RemoveMouseReleaseCallback(int idx)
{
	if (idx >= OnReleaseCallbacks.size())
		return;

	if (IsLiveReleaseCallbacks[idx])
	{
		IsLiveReleaseCallbacks[idx] = false;
		OnReleaseCallbacks[idx] = nullptr;
	}
}


