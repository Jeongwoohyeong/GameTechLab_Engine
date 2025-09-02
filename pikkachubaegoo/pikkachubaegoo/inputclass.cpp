#include "inputclass.h"
#include <Windows.h>

UInput::UInput()
{

}

UInput::~UInput()
{
}

UInput* UInput::GetInstance()
{
	static UInput instance;
	return &instance;
}

void UInput::Update()
{
	memcpy(PreviousKeyStates, CurrentKeyStates, sizeof(bool) * MAX_KEYS);

	for (int i = 0; i < MAX_KEYS; i++)
	{
		CurrentKeyStates[i] = GetAsyncKeyState(i) & 0x8000;
	}
}

bool UInput::IsKeyDown(int key)
{
	return CurrentKeyStates[key];
}

bool  UInput::IsPrevKeyDown(int key)
{
	return PreviousKeyStates[key];
}
bool  UInput::IsKeyPressed(int key)
{
	return IsKeyDown(key) && !IsPrevKeyDown(key);
}

bool  UInput::IsKeyReleased(int key)
{
	return IsPrevKeyDown(key) && !IsKeyDown(key);
}
