#include "PlayingState.h"
#include "App.h"
#include "ResultState.h"
#include "ObjectFactory.h"
#include <string>
#include "Time.h"
#include "Define.h"
#include "SoundManager.h"
#include "Background.h"

// 라운드를 시작하거나 재시작할 때 호출되는 함수
void PlayingState::ResetRound()
{
	UTime::GetInstance()->SetTimeScale(1.0f);
	player1->GetTransform()->SetLocation(FVector3(-PLAYER_BASE_POSITION_X, PLAYER_BASE_POSITION_Y)); // Player 1 위치 초기화
	player2->GetTransform()->SetLocation(FVector3(PLAYER_BASE_POSITION_X, PLAYER_BASE_POSITION_Y)); // Player 2 위치 초기화
	ball->GetTransform()->SetLocation(FVector3(BALL_BASE_POSITION_X, BALL_BASE_POSITION_Y)); // Ball 위치 초기화
	player1->SetVelocity(FVector3()); // Player1 속도 초기화
	player2->SetVelocity(FVector3()); // Player2 속도 초기화
	ball->SetVelocity(FVector3()); // Ball 속도 초기화

	// 상태를 'Ready'로 설정하고 타이머 초기화
	gameplayState = EGameplayState::Ready;
	stateTimer = 2.0f; // 카운트다운 전체 시간을 2초로 설정
}

void PlayingState::Enter()
{
	//배경 생성
	//bgSky = UObjectFactory::GetInstance()->CreateBG(UBackground::BGSkySpriteAtlasKey, UMeshRenderer::BGSkyOrder);
	//bgMountain = UObjectFactory::GetInstance()->CreateBG(UBackground::BGMountainSpriteAtlasKey, UMeshRenderer::BGMountainOrder);
	//bgGround = UObjectFactory::GetInstance()->CreateBG(UBackground::BGGroundSpriteAtlasKey, UMeshRenderer::BGGroundOrder);
	UObjectFactory::GetInstance()->CreateBG(UBackground::BGGroundSpriteAtlasKey, UMeshRenderer::BGGroundOrder);

	// 게임 플레이에 필요한 오브젝트들 생성
	player1 = UObjectFactory::GetInstance()->CreatePlayer(PLAYER1_INDEX, FVector3(-PLAYER_BASE_POSITION_X, PLAYER_BASE_POSITION_Y), FVector3(0.1f, 0.1f)); // Player 1, 왼쪽
	player2 = UObjectFactory::GetInstance()->CreatePlayer(PLAYER2_INDEX, FVector3(PLAYER_BASE_POSITION_X, PLAYER_BASE_POSITION_Y), FVector3(0.1f, 0.1f)); // Player 2, 오른쪽
	ball = UObjectFactory::GetInstance()->CreateBall(FVector3(BALL_BASE_POSITION_X, BALL_BASE_POSITION_Y)); // Ball
	UObjectFactory::GetInstance()->CreateWall(FVector3(WALL_BASE_POSITION_X, WALL_BASE_POSITION_Y), FVector3(WALL_SCALE_X, WALL_SCALE_Y)); // Wall

	player1Score = 0;
	player2Score = 0;

	ResetRound();
}

void PlayingState::Update(float deltaTime)
{
	switch (gameplayState)
	{
	case EGameplayState::Ready:
		// 카운트다운 로직
		stateTimer -= deltaTime;
		if (stateTimer <= 0.0f)
		{
			gameplayState = EGameplayState::InProgress;
		}
		break;

	case EGameplayState::InProgress:
		// 실제 게임 로직
		UObjectFactory::GetInstance()->Update(deltaTime);

		// 공이 바닥에 닿았을 때
		if (ball->GetPhysicsComponent()->IsGrounded())
		{
			USoundManager::GetInstance()->PlaySFX(SOUND_KEY_BALL_LAND);
			UObjectFactory::GetInstance()->CreatePunch(ball->GetTransform()->GetLocation(), FVector3(PUNCH_BALL_LAND_SCALE, PUNCH_BALL_LAND_SCALE));
			// 왼쪽이면 Player 2 승리, 오른쪽이면 Player 1 승리
			if (ball->GetTransform()->GetLocation().x < 0)
			{
				player2Score++;
			}
			else
			{
				player1Score++;
			}
			UTime::GetInstance()->SetTimeScale(0.3f);
			gameplayState = EGameplayState::RoundOver;
			stateTimer = 1.5f; // 1.5초간 결과 보여주기 (TimeScale에 영향 받지 않음)
		}
		break;

	case EGameplayState::RoundOver:
		// 라운드 종료 및 상태 전환 로직
		UObjectFactory::GetInstance()->Update(deltaTime);

		float timeScale = UTime::GetInstance()->GetTimeScale();
		// 0 나누기 방지
		if (timeScale != 0.0f)
		{
			// 타임스케일에 영향 받지 않도록 보정
			stateTimer -= deltaTime / timeScale;
		}

		if (stateTimer <= 0.0f)
		{
			// 최종 스코어 확인
			if (player1Score >= MAX_SCORE || player2Score >= MAX_SCORE)
			{
				// 게임 종료, 결과 상태로 전환
				UApp::Ins->ChangeState(new ResultState(player1Score, player2Score));
				gameplayState = EGameplayState::End;
			}
			else
			{
				// 아직 게임이 끝나지 않았으면 다음 라운드 시작
				ResetRound();
			}
		}
		break;
	}
}

void PlayingState::Render()
{
	// 게임 오브젝트 렌더링
	UObjectFactory::GetInstance()->Render();

	// --- UI 렌더링 ---

	// 1. 점수판 UI
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 200)); // 상단에 얇은 바
	ImGui::Begin("Scoreboard", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs);

	std::string scoreText = std::to_string(player1Score) + " : " + std::to_string(player2Score);
	float originalFontSize = ImGui::GetFont()->Scale;
	ImGui::GetFont()->Scale = 3.0f;
	ImGui::PushFont(ImGui::GetFont());

	ImVec2 textSize = ImGui::CalcTextSize(scoreText.c_str());
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - textSize.x) * 0.5f);
	ImGui::SetCursorPosY(10); // 상단에서 약간 아래
	ImGui::Text(scoreText.c_str());

	ImGui::PopFont();
	ImGui::GetFont()->Scale = originalFontSize;
	ImGui::End();

	// 세부 상태에 따른 UI 렌더링 (예: 카운트다운 숫자)
	if (gameplayState == EGameplayState::Ready)
	{
		// 1. 화면 전체를 덮는 투명한 창을 준비합니다. UI 요소를 게임 화면 위에 배치하기 위함입니다.
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		ImGui::Begin("CountdownUI", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs);

		// 2. 타이머 값에 따라 표시할 텍스트를 결정합니다.
		// 2. 타이머 값에 따라 표시할 텍스트를 결정합니다.
		std::string countdownText;
		if (stateTimer > 0.5f) // 남은 시간이 0.5초보다 많으면
		{
			countdownText = "Ready";
		}
		else // 남은 시간이 0.5초 이하이면
		{
			countdownText = "START!";
		}

		// 3. 폰트 크기를 일시적으로 키웁니다.
		float originalFontSize = ImGui::GetFont()->Scale;
		ImGui::GetFont()->Scale = 6.0f; // 6배 크기로 설정
		ImGui::PushFont(ImGui::GetFont()); // 변경된 폰트 적용

		// 4. 텍스트를 화면 중앙에 위치시키기 위한 계산
		ImVec2 textSize = ImGui::CalcTextSize(countdownText.c_str());
		ImVec2 windowSize = ImGui::GetWindowSize();
		ImGui::SetCursorPosX((windowSize.x - textSize.x) * 0.5f);
		ImGui::SetCursorPosY((windowSize.y - textSize.y) * 0.5f);

		// 5. 텍스트를 그립니다.
		ImGui::Text(countdownText.c_str());

		// 6. 폰트 크기를 원래대로 복원합니다. (매우 중요!)
		ImGui::PopFont();
		ImGui::GetFont()->Scale = originalFontSize;

		ImGui::End();
	}
}

void PlayingState::Exit()
{
	UObjectFactory::GetInstance()->ReleaseAll();

	player1 = nullptr;
	player2 = nullptr;
	ball = nullptr;
}