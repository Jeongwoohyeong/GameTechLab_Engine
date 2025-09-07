#include <Windows.h>
#include "CInputManager.h"

void CInputManager::Update()
{
	CopyMemory(PreviousKeyStates, CurrentKeyStates, sizeof(bool) * MAX_KEYS);

	for (int i = 0; i < MAX_KEYS; i++)
	{
		CurrentKeyStates[i] = GetAsyncKeyState(i) & 0x8000; //현재 눌려있는지 여부
	}
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
