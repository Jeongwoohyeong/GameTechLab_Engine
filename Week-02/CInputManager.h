#pragma once
#include <Windows.h>
#include <functional>
#include "Types.h"
#include "Math.h"

#define MAX_KEYS 256


struct FDragMouseData{
	int32 mouseX;
	int32 mouseY;
	int32 W;
	int32 H;
};

// 클릭/드래그/떼기 이벤트에 대한 콜백 함수 타입 정의
// 콜백 함수는 마우스 위치를 인자로 받을 수 있습니다.
using MouseCallback = std::function<void(FDragMouseData)>;
using MouseReleaseCallback = std::function<void()>;

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

	// 키 상태 확인 함수
	bool IsKeyDown(int32 key);     // 현재 눌려있는지
	bool IsPrevKeyDown(int32 key); // 이전 프레임에 눌려있었는지
	bool IsKeyPressed(int32 key);  // 이번 프레임에 막 눌렸는지
	bool IskeyReleased(int32 key); // 이번 프레임에 막 떼어졌는지

	// 마우스 위치 관련 함수
	POINT GetMouseClientPos();
	POINT GetMouseDelta() const;

	// 클라이언트 영역 크기 관련 함수
	int32 GetClientW() const;
	int32 GetClientH() const;

	// 이벤트 등록 함수
	void ReleaseCallbacks();

	int RegisterMouseClickCallback(MouseCallback callback);
	int RegisterMouseDragCallback(MouseCallback callback);
	int RegisterMouseReleaseCallback(MouseReleaseCallback callback);

	void RemoveMouseClickCallback(int idx);
	void RemoveMouseDragCallback(int idx);
	void RemoveMouseReleaseCallback(int idx);

	// 등록된 콜백 함수들을 저장할 리스트
	std::vector<MouseCallback> OnClickCallbacks;
	std::vector<MouseCallback> OnDragCallbacks;
	std::vector<MouseReleaseCallback> OnReleaseCallbacks;
	std::vector<bool> IsLiveClickCallbacks;
	std::vector<bool> IsLiveDragCallbacks;
	std::vector<bool> IsLiveReleaseCallbacks;

public:
	FVector MousePressPosWorld{0.0f, 0.0f, 0.0f}; // 마우스 클릭 시점의 Ray - 투영 평면 교차점 월드 좌표 (기즈모 드래그용)
	FVector MouseCurrentPosWorld{0.0f, 0.0f, 0.0f}; // 현재의 Ray - 투영 평면 교차점 월드 좌표 (기즈모 드래그용)

private:
	CInputManager() {};
	~CInputManager() {};
	void UpdateMousePosAndDelta();
	void UpdateClientSize();
	void UpdateKeyStates();	

	HWND* hWnd = nullptr;
	int32 ClientW = 0;
	int32 ClientH = 0;
	bool CurrentKeyStates[MAX_KEYS] = { 0 };
	bool PreviousKeyStates[MAX_KEYS] = { 0 };
	POINT CurrentMouseClientPosPoint = { 0, 0 };
	POINT PreviousMouseClientPosPoint = { 0, 0 };
};
