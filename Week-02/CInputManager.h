#pragma once
#include "Types.h"

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

	void Update();

	bool IsKeyDown(int32 key);

	bool IsPrevKeyDown(int32 key);

	bool IsKeyPressed(int32 key);

	bool IskeyReleased(int32 key);

private:
	CInputManager() {};
	~CInputManager() {};

	bool CurrentKeyStates[MAX_KEYS] = { 0 };
	bool PreviousKeyStates[MAX_KEYS] = { 0 };
};