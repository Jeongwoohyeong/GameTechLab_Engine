#include "PlayingState.h"
#include "App.h"
//#include "ResultState.h" // (가정) 결과 상태로 전환하기 위함
#include "ObjectFactory.h"
#include <string>

void PlayingState::Enter()
{
	// 게임 플레이에 필요한 오브젝트들 생성
	UObjectFactory::GetInstance()->CreatePlayer(FVector3(-0.5f)); // Player 1
	UObjectFactory::GetInstance()->CreatePlayer(FVector3(0.5f)); // Player 2
	UObjectFactory::GetInstance()->CreateBall(FVector3(0.0, 0.9f)); // Ball
	UObjectFactory::GetInstance()->CreateWall(FVector3(0.0f, -0.9f), FVector3(1.0f, 3.0f)); // Wall

	// 첫 세부 상태를 'Ready'로 설정
	gameplayState = EGameplayState::Ready;
	stateTimer = 3.0f; // 3초 카운트다운
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

		// 
		// (예시) 게임 종료 조건 확인
		//if (/* 누군가 승리했다면 */)
		//{
		//	gameplayState = EGameplayState::RoundOver;
		//	stateTimer = 2.0f; // 2초간 결과 보여주기
		//}
		break;

	case EGameplayState::RoundOver:
		// 라운드 종료 및 상태 전환 로직
		stateTimer -= deltaTime;
		if (stateTimer <= 0.0f)
		{
			// UApp에 ResultState로 전환 요청
			//UApp::Ins->ChangeState(new ResultState());
		}
		break;
	}
}

void PlayingState::Render()
{
	// 게임 오브젝트 렌더링
	UObjectFactory::GetInstance()->Render();

	// 세부 상태에 따른 UI 렌더링 (예: 카운트다운 숫자)
	if (gameplayState == EGameplayState::Ready)
	{
		// 1. 화면 전체를 덮는 투명한 창을 준비합니다. UI 요소를 게임 화면 위에 배치하기 위함입니다.
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		ImGui::Begin("CountdownUI", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs);

		// 2. 타이머 값에 따라 표시할 텍스트를 결정합니다.
		std::string countdownText;
		if (stateTimer > 0.0f)
		{
			// 타이머 값을 올림하여 정수로 만들고 문자열로 변환합니다. (e.g., 2.9초 -> "3")
			countdownText = std::to_string((int)ceil(stateTimer));
		}
		else
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
	// 생성했던 모든 게임 오브젝트 삭제
	for (auto& obj : gameObjects)
	{
		delete obj;
	}
	gameObjects.clear();
}