#include "pch.h"
#include "Render/UI/Widget/ActorTerminationWidget.h"
#include "Level/Level.h"
#include "Manager/Input/InputManager.h"
#include "Manager/Level/LevelManager.h"

IMPLEMENT_CLASS(UActorTerminationWidget, UWidget)

UActorTerminationWidget::UActorTerminationWidget() : SelectedActor(nullptr)
{
}

UActorTerminationWidget::~UActorTerminationWidget() = default;

void UActorTerminationWidget::Initialize()
{
	// Do Nothing Here
}

void UActorTerminationWidget::Update()
{
	ULevel* Level = ULevelManager::GetInstance().GetCurrentLevel();
	if (!Level)
	{
		SelectedActor = nullptr;
		return;
	}

	AActor* CurrSel = Level->GetSelectedActor();
	if (!Level->IsActorValid(CurrSel)) CurrSel = nullptr;

	if (SelectedActor != CurrSel)
	{
		SelectedActor = CurrSel;
	}
}

void UActorTerminationWidget::RenderWidget()
{
	auto& InputManager = UInputManager::GetInstance();

	// ImGui::Text("Actor Management");

	if (SelectedActor)
	{
		ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f), "Selected: %s (%p)",
		                   SelectedActor->GetName().c_str(), SelectedActor);

		if (ImGui::Button("Delete Selected") || InputManager.IsKeyDown(EKeyInput::Delete))
		{
			DeleteSelectedActor();
		}
	}
	else
	{
		// ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No Actor Selected For Deletion");
	}
}

/**
 * @brief Selected Actor 삭제 함수
 */
void UActorTerminationWidget::DeleteSelectedActor()
{
	if (!SelectedActor)
	{
		UE_LOG("ActorTerminationWidget: No Actor Selected For Deletion");
		return;
	}

	ULevelManager& LevelManager = ULevelManager::GetInstance();
	ULevel* CurrentLevel = LevelManager.GetCurrentLevel();

	if (!CurrentLevel)
	{
		UE_LOG("ActorTerminationWidget: No Current Level To Delete Actor From");
		return;
	}

	UE_LOG("ActorTerminationWidget: Marking Selected Actor For Deletion: %s (%p)",
	       SelectedActor->GetName().empty() ? "UnNamed" : SelectedActor->GetName().c_str(),
	       SelectedActor);

	// 지연 삭제를 사용하여 안전하게 다음 틱에서 삭제
	CurrentLevel->MarkActorForDeletion(SelectedActor);

	// MarkActorForDeletion에서 선택 해제도 처리하므로 여기에서는 단순히 nullptr로 설정
	SelectedActor = nullptr;
	UE_LOG("ActorTerminationWidget: Actor Marked For Deletion In Next Tick");
}
