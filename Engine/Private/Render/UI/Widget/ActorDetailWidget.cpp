#include "pch.h"
#include "Render/UI/Widget/ActorDetailWidget.h"

#include "Level/Level.h"
#include "Manager/Level/LevelManager.h"
#include "Actor/Actor.h"

UActorDetailWidget::UActorDetailWidget()
{
}

void UActorDetailWidget::Initialize()
{
	UE_LOG("ActorDetailWidget: Successfully Initialized");
}

void UActorDetailWidget::Update()
{
	ULevel* Level = ULevelManager::GetInstance().GetCurrentLevel();
	if (!Level)
	{
		SelectedActor = nullptr;
		ActorNameBuffer[0] = '\0';
		return;
	}

	AActor* CurrSel = Level->GetSelectedActor();
	if (!Level->IsActorValid(CurrSel))
	{
		CurrSel = nullptr;
	}

	if (SelectedActor != CurrSel)
	{
		SelectedActor = CurrSel;
		if (SelectedActor)
		{
			FString Name = SelectedActor->GetName();
			strncpy_s(ActorNameBuffer, Name.c_str(), sizeof(ActorNameBuffer) - 1);
			ActorNameBuffer[sizeof(ActorNameBuffer) - 1] = '\0';
		}
		else
		{
			ActorNameBuffer[0] = '\0';
		}
	}
}
// 위젯을 출력한다.
void UActorDetailWidget::RenderWidget()
{
	ImGui::Text("Actor Details");
	ImGui::Separator();

	if (SelectedActor)
	{
		RenderActorInfo();
		ImGui::Separator();
		RenderNameField();
	}
	else
	{
		ImGui::TextUnformatted("No Actor Selected");
	}

	if (bNameChanged && SelectedActor)
	{
		SelectedActor->SetName(FString(ActorNameBuffer));
		bNameChanged = false;
	}
}
// 액터의 정보들을 출력한다.
void UActorDetailWidget::RenderActorInfo()
{
	// 지금 순간의 레벨/선택을 '다시' 확인
	ULevel* Level = ULevelManager::GetInstance().GetCurrentLevel();
	if (!Level)
	{
		SelectedActor = nullptr;
		return;
	}
	// SelectedActor가 유효한지 다시 확인
	AActor* Actor = Level->IsActorValid(SelectedActor) ? SelectedActor : nullptr;
	if (!Actor)
	{
		SelectedActor = nullptr;
		return;
	}
	// Actor로만 사용
	UClass* ActorClass = Actor->GetClass();
	FString ClassName = ActorClass ? ActorClass->GetName() : "Unknown";

	ImGui::Text("Class: %s", ClassName.c_str());

	void* ActorPtr = static_cast<void*>(Actor);
	ImGui::Text("Address: %p", ActorPtr);

	uint64 MemoryUsage = Actor->GetAllocatedBytes();
	ImGui::Text("Memory: %llu bytes", MemoryUsage);
}

void UActorDetailWidget::RenderNameField()
{
	ImGui::Text("Name");
	if (ImGui::InputText("##ActorName", ActorNameBuffer, sizeof(ActorNameBuffer)))
	{
		bNameChanged = true;
	}
}
