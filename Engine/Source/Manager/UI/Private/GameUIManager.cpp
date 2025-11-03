#include "pch.h"
#include "Manager/UI/Public/GameUIManager.h"
#include "Render/UI/Widget/Public/GameStartMenuWidget.h"
#include "Render/UI/Widget/Public/GameHUDWidget.h"
#include "Render/UI/Widget/Public/GameOverWidget.h"
#include "Core/Public/NewObject.h"
#include "Editor/Public/EditorEngine.h"
#include "Manager/UI/Public/ViewportManager.h"
#include "GameMode/Public/GameModeBase.h"
#include "GamePlay/Public/PlayerController.h"
#include "GamePlay/Public/PlayerInput.h"
#include "Level/Public/World.h"
#include "Manager/Input/Public/InputManager.h"

IMPLEMENT_SINGLETON_CLASS(UGameUIManager, UObject)

UGameUIManager::UGameUIManager()
{
}

UGameUIManager::~UGameUIManager()
{
	Shutdown();
}

void UGameUIManager::Initialize()
{
	UE_LOG("[GameUIManager] Initializing...");

	// 위젯 생성
	CreateWidgets();

	// 초기 상태는 MainMenu (입력 비활성화)
	ShowMainMenu();

	UE_LOG("[GameUIManager] Initialized successfully");
}

void UGameUIManager::Shutdown()
{
	UE_LOG("[GameUIManager] Shutting down...");

	// 마우스 잠금 해제
	SetMouseLocked(false);

	// 위젯 정리
	if (StartMenuWidget)
	{
		delete StartMenuWidget;
		StartMenuWidget = nullptr;
	}

	if (HUDWidget)
	{
		delete HUDWidget;
		HUDWidget = nullptr;
	}

	if (GameOverWidget)
	{
		delete GameOverWidget;
		GameOverWidget = nullptr;
	}

	UE_LOG("[GameUIManager] Shutdown complete");
}

void UGameUIManager::Update()
{
	// 현재 상태에 따라 위젯 업데이트 및 버튼 이벤트 처리
	UpdateCurrentWidget();
}

void UGameUIManager::Render()
{
	// PIE 뷰포트 영역 가져오기
	float ViewportLeft, ViewportTop, ViewportWidth, ViewportHeight;
	GetPIEViewportRect(ViewportLeft, ViewportTop, ViewportWidth, ViewportHeight);

	// 뷰포트 영역을 각 위젯에 설정
	if (StartMenuWidget)
	{
		StartMenuWidget->SetViewportRect(ViewportLeft, ViewportTop, ViewportWidth, ViewportHeight);
	}
	if (HUDWidget)
	{
		HUDWidget->SetViewportRect(ViewportLeft, ViewportTop, ViewportWidth, ViewportHeight);
	}
	if (GameOverWidget)
	{
		GameOverWidget->SetViewportRect(ViewportLeft, ViewportTop, ViewportWidth, ViewportHeight);
	}

	// 현재 상태에 따라 적절한 위젯 렌더링
	switch (CurrentState)
	{
	case EGameUIState::MainMenu:
		if (StartMenuWidget)
		{
			StartMenuWidget->RenderWidget();
		}
		break;

	case EGameUIState::Playing:
		if (HUDWidget)
		{
			HUDWidget->RenderWidget();
		}
		break;

	case EGameUIState::GameOver:
		// 게임오버 시에는 HUD도 함께 표시 (최종 점수 확인용)
		if (HUDWidget)
		{
			HUDWidget->RenderWidget();
		}
		if (GameOverWidget)
		{
			GameOverWidget->RenderWidget();
		}
		break;
	}
}

void UGameUIManager::SetState(EGameUIState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	UE_LOG("[GameUIManager] State changed: %d -> %d", static_cast<int>(CurrentState), static_cast<int>(NewState));
	CurrentState = NewState;

	// 상태 변경 시 버튼 상태 리셋
	if (StartMenuWidget)
	{
		StartMenuWidget->ResetButtonStates();
	}
	if (GameOverWidget)
	{
		GameOverWidget->ResetButtonStates();
	}
}

void UGameUIManager::ShowMainMenu()
{
	SetState(EGameUIState::MainMenu);

	// 메뉴 상태: 마우스 잠금 해제 (커서 보임), 플레이어 입력 비활성화
	SetMouseLocked(false);
	SetPlayerInputEnabled(false);

	UE_LOG("[GameUIManager] Main Menu - Mouse unlocked, Input disabled");
}

void UGameUIManager::StartGame()
{
	SetState(EGameUIState::Playing);

	// HUD 초기화
	if (HUDWidget)
	{
		HUDWidget->Initialize();
	}

	// 게임 플레이 상태: 마우스 잠금 (커서 숨김), 플레이어 입력 활성화
	SetMouseLocked(true);
	SetPlayerInputEnabled(true);

	UE_LOG("[GameUIManager] Game Playing - Mouse locked, Input enabled");
}

void UGameUIManager::ShowGameOver(int32 FinalScore)
{
	SetState(EGameUIState::GameOver);

	// 최종 점수 설정
	if (GameOverWidget)
	{
		GameOverWidget->SetFinalScore(FinalScore);
	}

	// 게임 오버 상태: 마우스 잠금 해제 (커서 보임), 플레이어 입력 비활성화
	SetMouseLocked(false);
	SetPlayerInputEnabled(false);

	UE_LOG("[GameUIManager] Game Over - Mouse unlocked, Input disabled");
}

void UGameUIManager::CreateWidgets()
{
	// 시작 메뉴 위젯 생성
	StartMenuWidget = NewObject<UGameStartMenuWidget>(this);
	if (StartMenuWidget)
	{
		StartMenuWidget->Initialize();
		UE_LOG("[GameUIManager] StartMenuWidget created");
	}

	// HUD 위젯 생성
	HUDWidget = NewObject<UGameHUDWidget>(this);
	if (HUDWidget)
	{
		HUDWidget->Initialize();
		UE_LOG("[GameUIManager] HUDWidget created");
	}

	// 게임오버 위젯 생성
	GameOverWidget = NewObject<UGameOverWidget>(this);
	if (GameOverWidget)
	{
		GameOverWidget->Initialize();
		UE_LOG("[GameUIManager] GameOverWidget created");
	}
}

void UGameUIManager::UpdateCurrentWidget()
{
	switch (CurrentState)
	{
	case EGameUIState::MainMenu:
		if (StartMenuWidget)
		{
			StartMenuWidget->Update();

			// 버튼 이벤트 처리
			if (StartMenuWidget->IsStartButtonClicked())
			{
				StartMenuWidget->ResetButtonStates();
				StartGame();
				UE_LOG("[GameUIManager] Game started from main menu");
			}
			else if (StartMenuWidget->IsQuitButtonClicked())
			{
				StartMenuWidget->ResetButtonStates();
				// PIE 종료
				if (GEditor)
				{
					GEditor->EndPIE();
				}
				UE_LOG("[GameUIManager] Quit to editor");
			}
		}
		break;

	case EGameUIState::Playing:
		if (HUDWidget)
		{
			HUDWidget->Update();

			// 플레이어 사망 체크
			if (HUDWidget->IsPlayerDead())
			{
				ShowGameOver(HUDWidget->GetScore());
				UE_LOG("[GameUIManager] Player died, showing game over screen");
			}
		}
		break;

	case EGameUIState::GameOver:
		if (GameOverWidget)
		{
			GameOverWidget->Update();

			// 버튼 이벤트 처리
			if (GameOverWidget->IsRestartButtonClicked())
			{
				GameOverWidget->ResetButtonStates();
				// 재시작 = PIE 종료 후 다시 시작
				if (GEditor)
				{
					GEditor->EndPIE();
					GEditor->StartPIE();
				}
				UE_LOG("[GameUIManager] Restarting game");
			}
			else if (GameOverWidget->IsQuitButtonClicked())
			{
				GameOverWidget->ResetButtonStates();
				// 메인 메뉴로 돌아가기 = PIE 종료
				if (GEditor)
				{
					GEditor->EndPIE();
				}
				UE_LOG("[GameUIManager] Quit to main menu");
			}
		}
		break;
	}
}

void UGameUIManager::GetPIEViewportRect(float& OutLeft, float& OutTop, float& OutWidth, float& OutHeight)
{
	UViewportManager& ViewportMgr = UViewportManager::GetInstance();
	int32 PIEViewportIndex = ViewportMgr.GetPIEActiveViewportIndex();

	// PIE 뷰포트가 설정되어 있으면 해당 뷰포트의 영역 사용
	if (PIEViewportIndex >= 0)
	{
		TArray<FRect> ViewportRects;
		ViewportMgr.GetLeafRects(ViewportRects);

		if (PIEViewportIndex < ViewportRects.size())
		{
			const FRect& ViewportRect = ViewportRects[PIEViewportIndex];
			OutLeft = static_cast<float>(ViewportRect.Left);
			OutTop = static_cast<float>(ViewportRect.Top);
			OutWidth = static_cast<float>(ViewportRect.Width);
			OutHeight = static_cast<float>(ViewportRect.Height);
			return;
		}
	}

	// 폴백: 전체 화면 사용 (Single 뷰포트 모드)
	FRect ActiveRect = ViewportMgr.GetActiveViewportRect();
	OutLeft = static_cast<float>(ActiveRect.Left);
	OutTop = static_cast<float>(ActiveRect.Top);
	OutWidth = static_cast<float>(ActiveRect.Width);
	OutHeight = static_cast<float>(ActiveRect.Height);
}

UPlayerInput* UGameUIManager::GetPlayerInput()
{
	if (!GEditor || !GEditor->IsPIESessionActive())
	{
		return nullptr;
	}

	FWorldContext* PIEContext = GEditor->GetPIEWorldContext();
	if (!PIEContext || !PIEContext->World())
	{
		return nullptr;
	}

	AGameModeBase* GameMode = PIEContext->World()->GetGameMode();
	if (!GameMode)
	{
		return nullptr;
	}

	APlayerController* PlayerController = GameMode->GetPlayerController();
	if (!PlayerController)
	{
		return nullptr;
	}

	return PlayerController->GetPlayerInput();
}

void UGameUIManager::SetPlayerInputEnabled(bool bEnabled)
{
	UPlayerInput* PlayerInput = GetPlayerInput();
	if (PlayerInput)
	{
		PlayerInput->SetInputEnabled(bEnabled);
		UE_LOG("[GameUIManager] Player input %s", bEnabled ? "ENABLED" : "DISABLED");
	}
}

void UGameUIManager::SetMouseLocked(bool bLocked)
{
	UInputManager::GetInstance().LockMouseToCenter(bLocked);
	UE_LOG("[GameUIManager] Mouse %s", bLocked ? "LOCKED (hidden)" : "UNLOCKED (visible)");
}


