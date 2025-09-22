#include "pch.h"
#include "Level/Level.h"

#include "Actor/Actor.h"
#include "Components/TextComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Render/Renderer/Renderer.h"
#include "Utility/Metadata.h"
IMPLEMENT_CLASS(ULevel, UObject)

ULevel::ULevel() = default;

ULevel::ULevel(const FString& InName)
	: UObject(InName)
{
}

ULevel::~ULevel()
{
	Cleanup();
}

void ULevel::Init()
{
	// TEST CODE
}

void ULevel::Update()
{
	// Process Delayed Task
	ProcessPendingDeletions();

	uint64 AllocatedByte = GetAllocatedBytes();
	uint32 AllocatedCount = GetAllocatedCount();

	//LevelPrimitiveComponents.clear();
	StaticMeshComponentsToRender.clear();
	TextComponentsToRender.clear();
	AABBsToRender.clear();

	for (auto& Actor : LevelActors)
	{
		if (Actor)
		{
			Actor->Tick();
			GatherComponentsToRender(Actor);
		}
	}

}

void ULevel::Render()
{
}

void ULevel::Cleanup()
{
	SelectedActor = nullptr;

	// 지연 삭제 먼저
	for (AActor*& A : ActorsToDelete) {
		RemoveFromRenderQueues(A);
		SafeDelete(A);
	}
	ActorsToDelete.clear();

	// 모든 액터 삭제
	for (AActor*& A : LevelActors) {
		RemoveFromRenderQueues(A);
		SafeDelete(A);
	}
	LevelActors.clear();

	StaticMeshComponentsToRender.clear();
	TextComponentsToRender.clear();
	AABBsToRender.clear();
}

void ULevel::GatherComponentsToRender(AActor* Actor)
{
	if (!Actor) return;
	URenderer& Renderer = URenderer::GetInstance();
	for (auto& Component : Actor->GetOwnedComponents())
	{
		
		if (Component->IsA(UPrimitiveComponent::StaticClass()))
		{
			UPrimitiveComponent* PrimitiveComponent = static_cast<UPrimitiveComponent*>(Component);
			if (PrimitiveComponent->IsVisible())
			{
				PrimitiveComponent->AddToRenderList(this);
			}

			if (Renderer.IsShowFlagEnabled(EEngineShowFlags::SF_Bounds))
			{
				AABBsToRender.push_back(PrimitiveComponent->GetWorldBounds());
			}
		}
	}
}

//StaticMesh가 구현되면 주석 해제(09/19 13:05)
void ULevel::AddStaticMeshComponentToRender(UStaticMeshComponent* Component)
{
	StaticMeshComponentsToRender.push_back(Component);
}

void ULevel::AddTextComponentToRender(UTextComponent* Component)
{
	TextComponentsToRender.push_back(Component);
}

void ULevel::SetSelectedActor(AActor* InActor)
{

	SelectedActor = InActor;
	if (SelectedActor)
	{
		for (auto& Component : SelectedActor->GetOwnedComponents())
		{
			if (Component->GetComponentType() >= EComponentType::Primitive)
			{
				UPrimitiveComponent* PrimitiveComponent = static_cast<UPrimitiveComponent*>(Component);
				if (PrimitiveComponent->IsVisible())
				{
					PrimitiveComponent->SetColor({ 1.f, 0.8f, 0.2f, 0.4f });
				}
			}
		}
	}
}

/**
 * @brief Level에서 Actor 제거하는 함수
 */
bool ULevel::DestroyActor(AActor* InActor)
{
	if (!InActor)
	{
		return false;
	}
	RemoveFromRenderQueues(InActor);
	// LevelActors 리스트에서 제거
	for (auto Iterator = LevelActors.begin(); Iterator != LevelActors.end(); ++Iterator)
	{
		if (*Iterator == InActor)
		{
			LevelActors.erase(Iterator);
			break;
		}
	}


	// Remove Actor Selection
	if (SelectedActor == InActor)
	{
		SelectedActor = nullptr;
	}

	// Remove
	SafeDelete(InActor);

	UE_LOG("Level: Actor Destroyed Successfully");
	return true;
}

/**
 * @brief Delete In Next Tick
 */
void ULevel::MarkActorForDeletion(AActor* InActor)
{
	if (!InActor)
	{
		UE_LOG("Level: MarkActorForDeletion: InActor Is Null");
		return;
	}

	// 이미 삭제 대기 중인지 확인
	for (AActor* PendingActor : ActorsToDelete)
	{
		if (PendingActor == InActor)
		{
			UE_LOG("Level: Actor Already Marked For Deletion");
			return;
		}
	}
	RemoveFromRenderQueues(InActor);

	if (SelectedActor == InActor)
	{
		SelectedActor = nullptr;
	}
	// 삭제 대기 리스트에 추가
	ActorsToDelete.push_back(InActor);
	UE_LOG("Level: Actor Marked For Deletion In Next Tick: %p", InActor);
}

void ULevel::SaveCameraSnapshotFromCamera()
{
	if (!Camera) return;
	FCameraMetadata CameraMetadata;
	CameraMetadata.Location = Camera->GetLocation();
	CameraMetadata.Rotation = Camera->GetRotation(); // 쿼터니언이면 변환해서 저장
	CameraMetadata.Fov = Camera->GetFovY();
	CameraMetadata.Aspect = Camera->GetAspect();
	CameraMetadata.NearZ = Camera->GetNearZ();
	CameraMetadata.FarZ = Camera->GetFarZ();
	SavedCamera = CameraMetadata;
}

void ULevel::ApplySavedCameraSnapshotToCamera()
{
	if (!Camera) return;
	Camera->SetLocation(SavedCamera.Location);
	Camera->SetRotation(SavedCamera.Rotation); // 쿼터니언 API면 거기에 맞춰 Set
	Camera->SetFovY(SavedCamera.Fov);
	Camera->SetAspect(SavedCamera.Aspect);
	Camera->SetNearZ(SavedCamera.NearZ);
	Camera->SetFarZ(SavedCamera.FarZ);
}

/**
 * @brief Level에서 Actor를 실질적으로 제거하는 함수
 * 이전 Tick에서 마킹된 Actor를 제거한다
 */
void ULevel::ProcessPendingDeletions()
{
	if (ActorsToDelete.empty())
	{
		return;
	}

	UE_LOG("[Level] Processing %zu Pending Deletions", ActorsToDelete.size());

	// 대기 중인 액터들을 삭제
	for (AActor* ActorToDelete : ActorsToDelete)
	{
		if (!ActorToDelete)
			continue;

		// 혹시 남아있을 수 있는 참조 정리
		if (SelectedActor == ActorToDelete)
		{
			SelectedActor = nullptr;
			/*if (Gizmo)
			{
				Gizmo->SetTargetActor(nullptr);
			}*/
		}

		// LevelActors 리스트에서 제거
		for (auto Iterator = LevelActors.begin(); Iterator != LevelActors.end(); ++Iterator)
		{
			if (*Iterator == ActorToDelete)
			{
				LevelActors.erase(Iterator);
				break;
			}
		}

		// Release Memory
		SafeDelete(ActorToDelete);
		UE_LOG("[Level] Actor Deleted: %p", ActorToDelete);
	}

	// Clear TArray
	ActorsToDelete.clear();
	UE_LOG("[Level] All Pending Deletions Processed");
}

void ULevel::RemoveFromRenderQueues(AActor* Owner)
{
	if (!Owner) return;

	StaticMeshComponentsToRender.erase(
		std::remove_if(StaticMeshComponentsToRender.begin(), StaticMeshComponentsToRender.end(),
			[Owner](UStaticMeshComponent* C) { return C && C->GetOuter() == Owner; }),
		StaticMeshComponentsToRender.end());

	TextComponentsToRender.erase(
		std::remove_if(TextComponentsToRender.begin(), TextComponentsToRender.end(),
			[Owner](UTextComponent* C) { return C && C->GetOuter() == Owner; }),
		TextComponentsToRender.end());
}
