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
		FString NameString = SelectedActor->GetStaticMeshName();
		const char* CurrentMeshName = NameString.c_str();
		ImGui::ShowDemoWindow();
		
		//현재 선택된 Actor의 Mesh의 index
		static int CurrentIndex = -1;
		if (ImGui::BeginCombo("Static Mesh", CurrentMeshName))
		{
			TArray<FString> StaticMeshNameList;
			TArray<UStaticMesh*> StaticMeshList;
			bool bIndexIsFound = false;
			for (TObjectIterator<UStaticMesh> It; It; ++It)
			{
				UStaticMesh* StaticMesh = *It;
				if (StaticMesh)
				{
					StaticMeshNameList.push_back(StaticMesh->GetName());
					StaticMeshList.push_back(StaticMesh);
					//현재 선택된 Actor의 Mesh에 index를 부여(이게 없으면 새로 선택한 엑터의 매시리스트에서 기존에 선택한 매시가 하이라이팅됨.
					//그리고 리스트를 선택할때도 현재 매시가 하이라이팅 되야하는데 아래의 코드가 없으면 그게 안됨.
					//근데 매번 Iterator를 돌면서 배열을 채우고 문자열을 비교하는게 최선인지는 모르겠음.
					if (!bIndexIsFound && !StaticMesh->GetName().compare(CurrentMeshName))
					{
						CurrentIndex = StaticMeshNameList.Num() - 1;
						bIndexIsFound = true;  
					}
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
