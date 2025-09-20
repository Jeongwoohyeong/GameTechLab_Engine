#include "pch.h"
#include "Render/UI/Widget/TargetActorTransformWidget.h"

#include "Level/Level.h"
#include "Manager/Level/LevelManager.h"
#include "Core/ObjectIterator.h"
#include "Mesh/StaticMesh.h"

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
	ULevelManager& LevelManager = ULevelManager::GetInstance();
	ULevel* CurrentLevel = LevelManager.GetCurrentLevel();

	LevelMemoryByte = CurrentLevel->GetAllocatedBytes();
	LevelObjectCount = CurrentLevel->GetAllocatedCount();

	if (CurrentLevel)
	{
		AActor* CurrentSelectedActor = CurrentLevel->GetSelectedActor();

		// Update Current Selected Actor
		if (SelectedActor != CurrentSelectedActor)
		{
			SelectedActor = CurrentSelectedActor;
		}

		// Get Current Selected Actor Information
		if (SelectedActor)
		{
			UpdateTransformFromActor();
		}
		else if (CurrentSelectedActor)
		{
			SelectedActor = nullptr;
		}
	}
}

void UTargetActorTransformWidget::RenderWidget()
{
	// Level Memory Information
	ImGui::Text("Level Memory Information");
	ImGui::Text("Level Object Count: %s", to_string(LevelObjectCount).c_str());
	ImGui::Text("Level Memory Byte: %s", to_string(LevelMemoryByte).c_str());
	ImGui::Separator();

	ImGui::Text("Transform");

	if (SelectedActor)
	{
		bPositionChanged |= ImGui::DragFloat3("Location", &EditLocation.X, 0.1f);
		bRotationChanged |= ImGui::DragFloat3("Rotation", &EditRotation.X, 0.1f);

		// Uniform Scale 옵션
		bool bUniformScale = SelectedActor->IsUniformScale();
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
		SelectedActor->SetUniformScale(bUniformScale);



		//ImGui::ShowDemoWindow();
		ImGui::Separator();
		ImGui::Text("Static Mesh");
		const char* CurrentMeshName = SelectedActor->GetStaticMeshName().c_str();
		static int CurrentIndex = 0;
		


		if (ImGui::BeginCombo("Static Mesh", CurrentMeshName))
		{
			TArray<FString> StaticMeshNameList;
			TArray<UStaticMesh*> StaticMeshList;
			for (TObjectIterator<UStaticMesh> It; It; ++It)
			{
				UStaticMesh* StaticMesh = *It;
				if (StaticMesh)
				{
					StaticMeshNameList.push_back(StaticMesh->GetName());
					StaticMeshList.push_back(StaticMesh);
				}
			}
			for (int Index = 0; Index < StaticMeshNameList.Num(); Index++)
			{
				const bool bIsSelected = (CurrentIndex == Index);

				//선택되면 true, bool값이 true면 하이라이트
				if (ImGui::Selectable(StaticMeshNameList[Index].c_str(), bIsSelected))
				{
					if (CurrentIndex != Index)
					{
						CurrentIndex = Index;
						SelectedActor->SetStaticMesh(StaticMeshList[CurrentIndex]);
					}
					
				}

				
				if (bIsSelected)
					ImGui::SetItemDefaultFocus();

			}
			ImGui::EndCombo();	
		}
		

		
	}
	else
	{
		ImGui::TextUnformatted("Select Actor For Indicating");
	}

	ImGui::Separator();
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
	if (SelectedActor)
	{
		EditLocation = SelectedActor->GetActorLocation();
		EditRotation = SelectedActor->GetActorRotation();
		EditScale = SelectedActor->GetActorScale3D();
	}
}

void UTargetActorTransformWidget::ApplyTransformToActor() const
{
	if (SelectedActor)
	{
		SelectedActor->SetActorLocation(EditLocation);
		SelectedActor->SetActorRotation(EditRotation);
		SelectedActor->SetActorScale3D(EditScale);
	}
}
