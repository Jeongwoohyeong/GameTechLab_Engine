#pragma once
#include "Core/Public/Object.h"

class UGameStartMenuWidget;
class UGameHUDWidget;
class UGameOverWidget;

/**
 * @brief 게임 UI 상태
 */
enum class EGameUIState : uint8
{
	MainMenu,   // 메인 메뉴 (게임 시작 전)
	Playing,    // 게임 플레이 중
	GameOver    // 게임 오버
};

UCLASS()
class UGameUIManager : public UObject
{
	GENERATED_BODY()
	DECLARE_SINGLETON_CLASS(UGameUIManager, UObject)

public:
	void Initialize();
	void Shutdown();
	void Update();
	void Render();

	// 상태 관리
	void SetState(EGameUIState NewState);
	EGameUIState GetState() const { return CurrentState; }

	// HUD 데이터 접근
	UGameHUDWidget* GetHUDWidget() const { return HUDWidget; }

	// 상태 변경 (편의 함수)
	void ShowMainMenu();
	void StartGame();
	void ShowGameOver(int32 FinalScore);

private:
	void CreateWidgets();
	void UpdateCurrentWidget();
	void GetPIEViewportRect(float& OutLeft, float& OutTop, float& OutWidth, float& OutHeight);

	// PlayerInput 제어
	class UPlayerInput* GetPlayerInput();
	void SetPlayerInputEnabled(bool bEnabled);

	// 마우스 제어
	void SetMouseLocked(bool bLocked);

	// 현재 상태
	EGameUIState CurrentState = EGameUIState::MainMenu;

	// 위젯들
	UGameStartMenuWidget* StartMenuWidget = nullptr;
	UGameHUDWidget* HUDWidget = nullptr;
	UGameOverWidget* GameOverWidget = nullptr;
};
