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
		CurrentKeyStates[i] = GetAsyncKeyState(i) & 0x8000; //현재 눌려있는지 여부
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
	// 현재 눌려있고 이전에 눌린 적이 없다 = 처음 눌림
	return IsKeyDown(key) && !IsPrevKeyDown(key);
}

bool  UInput::IsKeyReleased(int key)
{
	// 이전에 눌려있고 현재 눌려있지 않다 = 키 떼어짐
	return IsPrevKeyDown(key) && !IsKeyDown(key);
}
