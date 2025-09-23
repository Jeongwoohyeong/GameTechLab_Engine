#include "pch.h"
#include "Render/UI/Widget/TargetActorTransformWidget.h"

#include "Level/Level.h"
#include "Manager/Level/LevelManager.h"
#include "Core/ObjectIterator.h"
#include "Mesh/StaticMesh.h"
#include "Mesh/Material.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextComponent.h"

IMPLEMENT_CLASS(UTargetActorTransformWidget, UWidget)
UTargetActorTransformWidget::UTargetActorTransformWidget()
{
}

UTargetActorTransformWidget::~UTargetActorTransformWidget() = default;

void UTargetActorTransformWidget::Initialize()
{
	// Do Nothing Here
}

void UTargetActorTransformWidget::Update()
{
	// 매 프레임 Level의 선택된 Actor를 확인해서 정보 반영
	ULevel* Level = ULevelManager::GetInstance().GetCurrentLevel();

	// Level 자체 null이면 전부 리셋하고 종료
	if (!Level)
	{
		SelectedActor = nullptr;
		LevelMemoryByte = 0;
		LevelObjectCount = 0;
		return;
	}

	LevelMemoryByte = Level->GetAllocatedBytes();
	LevelObjectCount = Level->GetAllocatedCount();

	// 레벨의 현재 선택 대상 동기화 (유효성 검사 포함)
	AActor* CurrSel = Level->GetSelectedActor();
	SelectedActor = Level->IsActorValid(CurrSel) ? CurrSel : nullptr;


	if (SelectedActor)
	{
		UpdateTransformFromActor();
	}
	else
	{
		// 선택 해제되면 편집 필드도 안전 값으로
		EditLocation = FVector::ZeroVector;
		EditRotation = FVector::ZeroVector;
		EditScale = FVector(1.f, 1.f, 1.f);
	}
}

void UTargetActorTransformWidget::RenderWidget()
{
	ULevel* Level = ULevelManager::GetInstance().GetCurrentLevel();
	AActor* Actor = (Level && Level->IsActorValid(SelectedActor)) ? SelectedActor : nullptr;
	if (!Actor)
	{
		return;
	}
	// Level Memory Information
	ImGui::Text("Level Memory Information");
	ImGui::Text("Level Object Count: %s", to_string(LevelObjectCount).c_str());
	ImGui::Text("Level Memory Byte: %s", to_string(LevelMemoryByte).c_str());
	ImGui::Separator();

	ImGui::Text("Transform");

	if (Actor)
	{
		bPositionChanged |= ImGui::DragFloat3("Location", &EditLocation.X, 0.1f);
		bRotationChanged |= ImGui::DragFloat3("Rotation", &EditRotation.X, 0.1f);

		// Uniform Scale 옵션
		bool bUniformScale = Actor->IsUniformScale();
		if (bUniformScale)
		{
			float UniformScale = EditScale.X;

			if (ImGui::DragFloat("Scale", &UniformScale, 0.01f, 0.01f, 10.0f))
			{
				EditScale = FVector(UniformScale, UniformScale, UniformScale);
				bScaleChanged = true;
			}
		}
		else
		{
			bScaleChanged |= ImGui::DragFloat3("Scale", &EditScale.X, 0.1f);
		}

		ImGui::Checkbox("Uniform Scale", &bUniformScale);
		Actor->SetUniformScale(bUniformScale);


		RenderComponents();
		ImGui::ShowDemoWindow();

		//현재 선택된 Actor의 Mesh의 index
		

		
	}
	else
	{
		ImGui::TextUnformatted("Select Actor For Indicating");
	}

	ImGui::Separator();
}

void UTargetActorTransformWidget::RenderComponents() 
{
	const TArray<UActorComponent*>& Components = SelectedActor->GetOwnedComponents();

	for (UActorComponent* Component : Components)
	{
		if (Component->IsA(UStaticMeshComponent::StaticClass()))
		{
			RenderStatitMeshComponent(static_cast<UStaticMeshComponent*>(Component));
		}
		else if (Component->IsA(UTextComponent::StaticClass()))
		{

		}
	}
	
}

void UTargetActorTransformWidget::RenderStatitMeshComponent(UStaticMeshComponent* Component) 
{
	if (ImGui::CollapsingHeader("Static Mesh##Header"))
	{

		const UStaticMesh* CurrentStaticMesh = Component->GetStaticMesh();

		const uint32 CurrentUUID = CurrentStaticMesh->GetUUID();

		if (ImGui::BeginCombo("Static Mesh##Combo", CurrentStaticMesh->GetAssetPathFileName().c_str()))
		{
			UpdateStaticMeshListCash();
			for (int Index = 0; Index < StaticMeshList.Num(); Index++)
			{
				const uint32 StaticMeshUUID = StaticMeshList[Index]->GetUUID();
				const bool bIsSelected = (CurrentUUID == StaticMeshUUID);

				//선택되면 true, bool값이 true면 하이라이트
				if (ImGui::Selectable(StaticMeshList[Index]->GetAssetPathFileName().c_str(), bIsSelected))
				{
					if (CurrentUUID != StaticMeshUUID)
					{
						Component->SetStaticMesh(StaticMeshList[Index]);
					}
				}

				if (bIsSelected)
					ImGui::SetItemDefaultFocus();

			}
			ImGui::EndCombo();
		}
	}
	//Collapsing 헤더가 닫히는 순간부터는 추적이 안되므로 true(헤더를 여는 순간만 1회 업데이트 할 것임)
	//헤더를 연 상태에서 StaticMeshList가 업데이트 되면 반영 안 됨. 다시 열어야 함.
	bStaticMeshListDirty = true;
	RenderMaterials(Component);
	
}

void UTargetActorTransformWidget::RenderMaterials(UStaticMeshComponent* Component) 
{
	if (ImGui::CollapsingHeader("Materials##Of StaticMesh"))
	{
		UpdateMaterialListCash();
		const TArray<UMaterial*>& MaterialListOfComponent = Component->GetMaterialList();
		if (!MaterialListOfComponent[0])
			return;
		for (int Index = 0; Index < MaterialListOfComponent.Num(); Index++)
		{
			const FString& CurrentMaterialName = MaterialListOfComponent[Index]->GetMaterialName();
			
			const uint32 CurrentUUID = MaterialListOfComponent[Index]->GetUUID();

			FString Tag = FString("Material ").append(std::to_string(Index));
			if (ImGui::BeginCombo(Tag.c_str(), CurrentMaterialName.c_str()))
			{
				//프로그램 내의 모든 Material에 대한 Index
				for (int WIndex = 0; WIndex < MaterialList.Num(); WIndex++)
				{
					const uint32 MaterialUUID = MaterialList[WIndex]->GetUUID();
					const bool bIsSelected = (CurrentUUID == MaterialUUID);

					//선택되면 true, bool값이 true면 하이라이트
					const FString& MaterialName = MaterialList[WIndex]->GetMaterialName();
					if (MaterialName.empty())
						continue;
					if (ImGui::Selectable(MaterialName.c_str(), bIsSelected))
					{
						if (CurrentUUID != MaterialUUID)
						{
							Component->SetMaterial(MaterialList[WIndex], Index);
						}
					}

					if (bIsSelected)
						ImGui::SetItemDefaultFocus();

				}
				ImGui::EndCombo();
			}
		}
	}
	//Collapsing 헤더가 닫히는 순간부터는 추적이 안되므로 true(헤더를 여는 순간만 1회 업데이트 할 것임)
	//헤더를 연 상태에서 MaterialList가 업데이트 되면 반영 안 됨. 다시 열어야 함.
	bMaterialListDirty = true;
}

void UTargetActorTransformWidget::UpdateMaterialListCash()
{
	if (bMaterialListDirty)
	{
		MaterialList.clear();
		for (TObjectIterator<UMaterial> It; It; ++It)
		{
			UMaterial* Material = *It;
			if (Material)
			{
				MaterialList.push_back(Material);
			}
		}
	}
	bMaterialListDirty = false;
}

void UTargetActorTransformWidget::UpdateStaticMeshListCash()
{

	if (bStaticMeshListDirty)
	{
		StaticMeshList.clear();
		for (TObjectIterator<UStaticMesh> It; It; ++It)
		{
			UStaticMesh* StaticMesh = *It;
			if (StaticMesh)
			{
				StaticMeshList.push_back(StaticMesh);
			}
		}
	}
	bStaticMeshListDirty = false;
}

/**
 * @brief Render에서 체크된 내용으로 인해 이후 변경되어야 할 내용이 있다면 Change 처리
 */
void UTargetActorTransformWidget::PostProcess()
{
	if (bPositionChanged || bRotationChanged || bScaleChanged)
	{
		ApplyTransformToActor();
	}
}

void UTargetActorTransformWidget::UpdateTransformFromActor()
{
	ULevel* Level = ULevelManager::GetInstance().GetCurrentLevel();
	AActor* Actor = (Level && Level->IsActorValid(SelectedActor)) ? SelectedActor : nullptr;
	if (!Actor)
	{
		return;
	}
	if (Actor)
	{
		EditLocation = Actor->GetActorLocation();
		EditRotation = Actor->GetActorRotation();
		EditScale = Actor->GetActorScale3D();
	}
}

void UTargetActorTransformWidget::ApplyTransformToActor() const
{
	ULevel* Level = ULevelManager::GetInstance().GetCurrentLevel();
	AActor* Actor = (Level && Level->IsActorValid(SelectedActor)) ? SelectedActor : nullptr;
	if (!Actor)
	{
		return;
	}
	if (Actor)
	{
		Actor->SetActorLocation(EditLocation);
		Actor->SetActorRotation(EditRotation);
		Actor->SetActorScale3D(EditScale);
	}
}
