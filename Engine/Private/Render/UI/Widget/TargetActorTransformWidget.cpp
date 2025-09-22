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



		//ImGui::ShowDemoWindow();
		ImGui::Separator();
		ImGui::Text("Static Mesh");
		FString NameString = Actor->GetStaticMeshName();
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
					StaticMeshNameList.push_back(StaticMesh->GetAssetPathFileName());
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
						Actor->SetStaticMesh(StaticMeshList[CurrentIndex]);
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
