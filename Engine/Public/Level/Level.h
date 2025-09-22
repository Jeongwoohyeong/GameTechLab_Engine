#pragma once
#include "Core/Object.h"

#include "Editor/Camera.h"
#include "Utility/Metadata.h"
class AAxis;
class AGizmo;
class AGrid;
class AActor;
class UPrimitiveComponent;
class UStaticMeshComponent;
class UTextComponent;
struct FAABB;

class ULevel : public UObject
{
	DECLARE_CLASS(ULevel, UObject)
public:
	ULevel();
	ULevel(const FString& InName);
	~ULevel() override;

	virtual void Init();
	virtual void Update();
	virtual void Render();
	virtual void Cleanup();

	void GatherComponentsToRender(AActor* Actor);

	TArray<AActor*> GetLevelActors() const
	{
		return LevelActors;
	}
	const TArray<FAABB>& GetAABBsToRender() const
	{
		return AABBsToRender;
	}
	const TArray<UStaticMeshComponent*>& GetStaticMeshComponentsToRender() const
	{
		return StaticMeshComponentsToRender;
	}
	const TArray<UTextComponent*>& GetTextComponentsToRender() const
	{
		return TextComponentsToRender;
	}
	AActor* GetSelectedActor() const
	{
		return SelectedActor;
	}

	//StaticMesh가 구현되면 주석 해제(09/19 13:05)
	void AddStaticMeshComponentToRender(UStaticMeshComponent* Component);
	void AddTextComponentToRender(UTextComponent* Component);

	template<typename T, typename... Args>
	T* SpawnActor(const FString& InName = "");

	// 선택된 액터 설정
	void SetSelectedActor(AActor* InActor);
	// Actor 삭제
	bool DestroyActor(AActor* InActor);
	// 지연 삭제를 위한 마킹
	void MarkActorForDeletion(AActor* InActor);


	UCamera* GetCamera() const
	{
		return Camera;
	}
	void SetCamera(UCamera* InCamera)
	{
		Camera = InCamera;
	}

	// ▼ 카메라 스냅샷 저장/적용
	void SaveCameraSnapshotFromCamera();
	void ApplySavedCameraSnapshotToCamera();
	void SetSavedCameraSnapshot(const FCameraMetadata& In)
	{
		SavedCamera = In;
	}
	const FCameraMetadata& GetSavedCameraSnapshot() const
	{
		return SavedCamera;
	}
private:
	// 지연 삭제 처리 함수
	void ProcessPendingDeletions();
	// 렌더 큐에서 해당 액터 컴포넌트 제거
	void RemoveFromRenderQueues(AActor* Owner);  
private:
	TArray<AActor*> LevelActors;

	UCamera* Camera = nullptr;          // 비소유
	FCameraMetadata SavedCamera;        // 직렬화 대상으로 보관

	//렌더러에게 아래의 것들을 그려달라고 주문할 거임

	TArray<UStaticMeshComponent*> StaticMeshComponentsToRender;
	TArray<FAABB> AABBsToRender;
	TArray<UTextComponent*> TextComponentsToRender;


	// 지연 삭제를 위한 리스트
	TArray<AActor*> ActorsToDelete;

	AActor* SelectedActor = nullptr;


};

template <typename T, typename ... Args>
T* ULevel::SpawnActor(const FString& InName)
{
	T* NewActor = NewObject<T>();

	///////////////////////////////////////////
	NewActor->AddMemoryUsage(sizeof(T));
	///생성자에서 자신의 메모리 설정하게 수정 필요///
	NewActor->SetOuter(this);
	//Outer 설정 시 Outer의 메모리 카운트에 자신의 메모리 합산 작업 수행

	LevelActors.push_back(NewActor);
	if (!InName.empty())
	{
		NewActor->SetName(InName);
	}
	NewActor->BeginPlay();

	UE_LOG("%s", NewActor->GetName().c_str());

	return NewActor;
}
