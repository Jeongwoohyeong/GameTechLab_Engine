#include "pch.h"
#include "TargetActorTransformWidget.h"
#include "UI/UIManager.h"
#include "ImGui/imgui.h"
#include "Actor.h"
#include "World.h"
#include "Vector.h"
#include "GizmoActor.h"

#include "BillboardComponent.h"
#include "StaticMeshActor.h"    
#include "StaticMeshComponent.h"
#include "ResourceManager.h"    
#include "SceneComponent.h"    
#include "TextRenderComponent.h"    
#include "DecalComponent.h"
#include "SpotlightComponent.h"
#include "RotationMovementComponent.h"
#include "ProjectileMovementComponent.h"
#include "HeightFogComponent.h"
#include "FireBallComponent.h"

#include <string>
#include <filesystem>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

//// UE_LOG 대체 매크로
//#define UE_LOG(fmt, ...)

// 파일명 스템(Cube 등) 추출 + .obj 확장자 제거
static inline FString GetBaseNameNoExt(const FString& Path)
{
	const size_t sep = Path.find_last_of("/\\");
	const size_t start = (sep == FString::npos) ? 0 : sep + 1;

	const FString ext = ".obj";
	size_t end = Path.size();
	if (end >= ext.size() && Path.compare(end - ext.size(), ext.size(), ext) == 0)
	{
		end -= ext.size();
	}
	if (start <= end) return Path.substr(start, end - start);
	return Path;
}

// Editor/Icon 폴더에서 모든 .dds 파일을 동적으로 찾아서 반환
static TArray<FString> GetIconFiles()
{
	TArray<FString> iconFiles;
	try
	{
		fs::path iconPath = "Editor/Icon";
		if (fs::exists(iconPath) && fs::is_directory(iconPath))
		{
			for (const auto& entry : fs::directory_iterator(iconPath))
			{
				if (entry.is_regular_file())
				{
					auto filename = entry.path().filename().string();
					// .dds 확장자만 포함
					if (filename.ends_with(".dds"))
					{
						// 상대경로 포맷으로 저장 (Editor/Icon/filename.dds)
						FString relativePath = "Editor/Icon/" + filename;
						iconFiles.push_back(relativePath);
					}
				}
			}
		}
	}
	catch (const std::exception&)
	{
		// 파일 시스템 오류 발생 시 기본값으로 폴백
		iconFiles.push_back("Editor/Icon/Pawn_64x.dds");
		iconFiles.push_back("Editor/Icon/PointLight_64x.dds");
		iconFiles.push_back("Editor/Icon/SpotLight_64x.dds");
	}
	return iconFiles;
}

static TArray<FString> GetDecalFiles()
{
	TArray<FString> DecalFiles;
	try
	{
		fs::path DecalPath = "Editor/Decal";
		if (fs::exists(DecalPath) && fs::is_directory(DecalPath))
		{
			for (const auto& entry : fs::directory_iterator(DecalPath))
			{
				if (entry.is_regular_file())
				{
					auto Filename = entry.path().filename().string();
					if (Filename.ends_with(".dds"))
					{
						FString RelativePath = "Editor/Decal/" + Filename;
						DecalFiles.push_back(RelativePath);
					}
				}
			}
		}
	}
	catch (const std::exception&)
	{
		DecalFiles.push_back("Editor/Decal/Pawn_64x.dds");
		DecalFiles.push_back("Editor/Decal/PointLight_64x.dds");
		DecalFiles.push_back("Editor/Decal/SpotLight_64x.dds");
	}

	return DecalFiles;
}

UTargetActorTransformWidget::UTargetActorTransformWidget()
	: UWidget("Target Actor Transform Widget")
	, UIManager(&UUIManager::GetInstance())
{

}

UTargetActorTransformWidget::~UTargetActorTransformWidget() = default;

void UTargetActorTransformWidget::OnSelectedActorCleared()
{
	// 즉시 내부 캐시/플래그 정리
	SelectedActor = nullptr;
	CachedActorName.clear();
	ResetChangeFlags();
}

void UTargetActorTransformWidget::Initialize()
{
	// UIManager 참조 확보
	UIManager = &UUIManager::GetInstance();

	// Transform 위젯을 UIManager에 등록하여 선택 해제 브로드캐스트를 받을 수 있게 함
	if (UIManager)
	{
		UIManager->RegisterTargetTransformWidget(this);
	}

	// 추가 가능한 컴포넌트 타입 목록 초기화 (메타데이터 기반)
	if (!bComponentTypesInitialized)
	{
		// ObjectFactory의 NameRegistry를 순회하면서 메타데이터를 체크
		for (const auto& ClassDataPair : ObjectFactory::GetNameRegistry())
		{
			UClass* ClassType = ClassDataPair.second;

			if (!ClassType)
				continue;
			// 메타데이터를 체크하여 TargetActorTransformWidget에서 생성 가능한지 확인
			if (!ClassType->GetMetaDataBool("CanSpawnInTransformWidget", false))
				continue;


			// USceneComponent의 파생 클래스인지 확인
			if (ClassType->IsChildOf(USceneComponent::StaticClass()))
			{
				// 리스트에 추가
				FString DisplayName = ClassDataPair.first;
				AddableSceneComponentTypes.push_back({ DisplayName, ClassType });
			}
			else if (ClassType->IsChildOf(UActorComponent::StaticClass()))
			{
				// 리스트에 추가
				FString DisplayName = ClassDataPair.first;
				AddableNonSceneComponentTypes.push_back({ DisplayName, ClassType });
			}
		}
		bComponentTypesInitialized = true;
	}
}

AActor* UTargetActorTransformWidget::GetCurrentSelectedActor() const
{
	if (!UIManager)
		return nullptr;

	return UIManager->GetSelectedActor();
}

void UTargetActorTransformWidget::Update()
{
	// UIManager를 통해 현재 선택된 액터 가져오기
	AActor* CurrentSelectedActor = GetCurrentSelectedActor();
	if (SelectedActor != CurrentSelectedActor)
	{
		SelectedActor = CurrentSelectedActor;
		// 새로 선택된 액터의 이름 캐시
		if (SelectedActor)
		{
			try
			{
				// 새로운 액터가 선택되면, 선택된 컴포넌트를 해당 액터의 루트 컴포넌트로 초기화합니다.
				SelectedComponent = SelectedActor->GetRootComponent();

				CachedActorName = SelectedActor->GetName().ToString();
			}
			catch (...)
			{
				CachedActorName = "[Invalid Actor]";
				SelectedActor = nullptr;
				SelectedComponent = nullptr;
			}
		}
		else
		{
			CachedActorName = "";
			SelectedComponent = nullptr;
		}
	}

	if (SelectedActor)
	{
		// 액터가 선택되어 있으면 항상 트랜스폼 정보를 업데이트하여
		// 기즈모 조작을 실시간으로 UI에 반영합니다.
		UpdateTransformFromActor();
	}
}

/**
 * @brief Actor 복제 테스트 함수
 */
void UTargetActorTransformWidget::DuplicateTarget() const
{
	if (SelectedActor)
	{
		AActor* NewActor = Cast<AActor>(SelectedActor->Duplicate());
		
		// 초기 트랜스폼 적용
		NewActor->SetActorTransform(SelectedActor->GetActorTransform());

		// TODO(KHJ): World 접근?
		UWorld* World = SelectedActor->GetWorld();
		
		World->SpawnActor(NewActor);
	}
}

void UTargetActorTransformWidget::RenderWidget()
{
	if (SelectedActor)
	{
		// 액터 이름 표시 (캐시된 이름 사용)
		ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Selected: %s", CachedActorName.c_str());
		// 선택된 액터 UUID 표시(전역 고유 ID)
		ImGui::Text("UUID: %u", static_cast<unsigned int>(SelectedActor->UUID));
		ImGui::Spacing();

		// 선택된 Component가 USceneComponent인 경우 사용
		USceneComponent* SceneComponent = Cast<USceneComponent>(SelectedComponent);

		// 컴포넌트 추가 메뉴
		if (ImGui::Button("+추가"))
		{
			ImGui::OpenPopup("AddComponentPopup");
		}

		ImGui::SameLine();

		if (ImGui::Button("-삭제"))
		{
			if (SelectedComponent && SceneComponent)
			{
				// 컴포넌트 삭제 시 상위 컴포넌트로 선택되도록 설정
				USceneComponent* ParentComponent = SceneComponent->GetAttachParent();
				if (SelectedActor->DeleteSceneComponent(SceneComponent))
				{
					// 삭제 직후 SelectedComponent를 즉시 업데이트
					if (ParentComponent)
					{
						SelectedComponent = ParentComponent;
					}
					else
					{
						SelectedComponent = SelectedActor->GetRootComponent();
					}

					UpdateTransformFromActor();
				}
			}
			else
			{
				SelectedActor->DeleteNonSceneComponent(SelectedComponent);
				// 비계층 컴포넌트 삭제 시 자동으로 루트를 선택 컴포넌트로 등록한다.
				SelectedComponent = SelectedActor->GetRootComponent();

				UpdateTransformFromActor();
			}
		}

		// "Add Component" 버튼에 대한 팝업 메뉴 정의
		if (ImGui::BeginPopup("AddComponentPopup"))
		{
			ImGui::BeginChild("ComponentListScroll", ImVec2(200.0f, 150.0f), true);

			if (SelectedComponent && SceneComponent)
			{
				// 추가 가능한 컴포넌트 타입 목록 메뉴 표시
				for (const TPair<FString, UClass*>& Item : AddableSceneComponentTypes)
				{
					if (ImGui::Selectable(Item.first.c_str()))
					{
						// 컴포넌트를 누르면 생성 함수를 호출합니다.
						USceneComponent* NewSceneComponent = SelectedActor->CreateAndAttachComponent(SceneComponent, Item.second);
						// SelectedComponent를 생성된 컴포넌트로 교체합니다
						SceneComponent = NewSceneComponent;
						ImGui::CloseCurrentPopup();
					}
				}
			}

			ImGui::NewLine();
			ImGui::Separator();
			ImGui::NewLine();

			// 비계층 컴포넌트는 항상 추가할 수 있다.
			for (const TPair<FString, UClass*>& Item : AddableNonSceneComponentTypes)
			{
				if (ImGui::Selectable(Item.first.c_str()))
				{
					// 컴포넌트를 누르면 생성 함수를 호출합니다.
					UActorComponent* NewSceneComponent = Cast<UActorComponent>(NewObject(Item.second));
					SelectedActor->AddNonSceneComponent(NewSceneComponent);
					// SelectedComponent를 생성된 컴포넌트로 교체합니다
					SelectedComponent = NewSceneComponent;
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndChild();
			ImGui::EndPopup();
		}

		// 컴포넌트 계층 구조 표시
		ImGui::BeginChild("ComponentHierarchy", ImVec2(0, 240), true);
		const FName ActorName = SelectedActor->GetName();

		// 1. 최상위 액터 노드는 클릭해도 접을 수 없습니다.
		ImGui::TreeNodeEx(
			ActorName.ToString().c_str(),
			ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen
		);

		// 2. 수동으로 들여쓰기를 추가합니다.
		ImGui::Indent();

		// 3. 하위 컴포넌트를 조건 없이 항상 그립니다.
		USceneComponent* RootComponent = SelectedActor->GetRootComponent();
		if (RootComponent)
		{
			RenderComponentHierarchy(RootComponent);
		}

		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();

		// 비계층 컴포넌트 목록 표시
		TArray<UActorComponent*> OwnedNonSceneComponents = SelectedActor->GetOwnedNonSceneComponent();
		for (UActorComponent* NonSceneComponent : OwnedNonSceneComponents)
		{
			FString ComponentName = NonSceneComponent->GetName().ToString();
			//FString ComponentClass = NonSceneComponent->GetClass()->Name;
			FString DisplayText = ComponentName;

			// 선택 가능한 항목으로 표시
			bool bIsSelected = (SelectedComponent == NonSceneComponent);
			if (ImGui::Selectable(DisplayText.c_str(), bIsSelected))
			{
				// 클릭하면 선택
				SelectedComponent = NonSceneComponent;
			}
		}

		// 4. 들여쓰기를 해제합니다.
		ImGui::Unindent();
		ImGui::EndChild();

		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();

		if (SceneComponent)
		{
			// Location 편집
			if (ImGui::DragFloat3("Location", &EditLocation.X, 0.1f))
			{
				bPositionChanged = true;
			}

			// Rotation 편집 (Euler angles)
			if (ImGui::DragFloat3("Rotation", &EditRotation.X, 0.5f))
			{
				bRotationChanged = true;
			}

			// Scale 편집
			ImGui::Checkbox("Uniform Scale", &bUniformScale);

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
				if (ImGui::DragFloat3("Scale", &EditScale.X, 0.01f, 0.01f, 10.0f))
				{
					bScaleChanged = true;
				}
			}

			ImGui::Spacing();

			// 실시간 적용 버튼
			if (ImGui::Button("Apply Transform"))
			{
				ApplyTransformToActor();
			}

			ImGui::SameLine();
			if (ImGui::Button("Reset Transform"))
			{
				UpdateTransformFromActor();
				ResetChangeFlags();
			}

			ImGui::Spacing();
			ImGui::Separator();
		}
		
		// NOTE: 추후 컴포넌트별 위젯 따로 추가
		// Actor가 AStaticMeshActor인 경우 StaticMesh 변경 UI
		if (SelectedComponent)
		{
			if (UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(SelectedComponent))
			{
				ImGui::Text("Static Mesh Override");
				if (!SMC)
				{
					ImGui::TextColored(ImVec4(1, 0.6f, 0.6f, 1), "StaticMeshComponent not found.");
				}
				else
				{
					// 현재 메시 경로 표시
					FString CurrentPath;
					UStaticMesh* CurMesh = SMC->GetStaticMesh();
					if (CurMesh)
					{
						CurrentPath = CurMesh->GetAssetPathFileName();
						ImGui::Text("Current: %s", CurrentPath.c_str());
					}
					else
					{
						ImGui::Text("Current: <None>");
					}

					// 리소스 매니저에서 로드된 모든 StaticMesh 경로 수집
					auto& RM = UResourceManager::GetInstance();
					TArray<FString> Paths = RM.GetAllStaticMeshFilePaths();

					if (Paths.empty())
					{
						ImGui::TextColored(ImVec4(1, 0.6f, 0.6f, 1), "No StaticMesh resources loaded.");
					}
					else
					{
						// 표시용 이름(파일명 스템)
						TArray<FString> DisplayNames;
						DisplayNames.reserve(Paths.size());
						for (const FString& p : Paths)
							DisplayNames.push_back(GetBaseNameNoExt(p));

						// ImGui 콤보 아이템 배열
						TArray<const char*> Items;
						Items.reserve(DisplayNames.size());
						for (const FString& n : DisplayNames)
							Items.push_back(n.c_str());

						// 선택 인덱스 유지
						static int SelectedMeshIdx = -1;

						// 기본 선택: Cube가 있으면 자동 선택
						if (SelectedMeshIdx == -1)
						{
							for (int i = 0; i < static_cast<int>(Paths.size()); ++i)
							{
								if (DisplayNames[i] == "Cube" || Paths[i] == "Data/Cube.obj")
								{
									SelectedMeshIdx = i;
									break;
								}
							}
						}

						ImGui::SetNextItemWidth(240);
						ImGui::Combo("StaticMesh", &SelectedMeshIdx, Items.data(), static_cast<int>(Items.size()));
						ImGui::SameLine();
						if (ImGui::Button("Apply Mesh"))
						{
							if (SelectedMeshIdx >= 0 && SelectedMeshIdx < static_cast<int>(Paths.size()))
							{
								const FString& NewPath = Paths[SelectedMeshIdx];
								SMC->SetStaticMesh(NewPath);

								// Sphere 충돌 특례
								if (AStaticMeshActor* SMActor = Cast<AStaticMeshActor>(SelectedActor))
								{
									if (GetBaseNameNoExt(NewPath) == "Sphere")
										SMActor->SetCollisionComponent(EPrimitiveType::Sphere);
									else
										SMActor->SetCollisionComponent();
								}

								UE_LOG("Applied StaticMesh: %s", NewPath.c_str());
							}
						}

						// 현재 메시로 선택 동기화 버튼 (옵션)
						ImGui::SameLine();
						if (ImGui::Button("Select Current"))
						{
							SelectedMeshIdx = -1;
							if (!CurrentPath.empty())
							{
								for (int i = 0; i < static_cast<int>(Paths.size()); ++i)
								{
									if (Paths[i] == CurrentPath ||
										DisplayNames[i] == GetBaseNameNoExt(CurrentPath))
									{
										SelectedMeshIdx = i;
										break;
									}
								}
							}
						}
					}

					// Material 설정

					const TArray<FString> MaterialNames = UResourceManager::GetInstance().GetAllFilePaths<UMaterial>();
					// ImGui 콤보 아이템 배열
					TArray<const char*> MaterialNamesCharP;
					MaterialNamesCharP.reserve(MaterialNames.size());
					for (const FString& n : MaterialNames)
						MaterialNamesCharP.push_back(n.c_str());

					if (CurMesh)
					{
						const uint64 MeshGroupCount = CurMesh->GetMeshGroupCount();

						if (0 < MeshGroupCount)
						{
							ImGui::Separator();
						}

						static TArray<int32> SelectedMaterialIdxAt; // i번 째 Material Slot이 가지고 있는 MaterialName이 MaterialNames의 몇번쩨 값인지.
						if (SelectedMaterialIdxAt.size() < MeshGroupCount)
						{
							SelectedMaterialIdxAt.resize(MeshGroupCount);
						}

						// 현재 SMC의 MaterialSlots 정보를 UI에 반영
						const TArray<FMaterialSlot>& MaterialSlots = SMC->GetMaterailSlots();
						for (uint64 MaterialSlotIndex = 0; MaterialSlotIndex < MeshGroupCount; ++MaterialSlotIndex)
						{
							for (uint32 MaterialIndex = 0; MaterialIndex < MaterialNames.size(); ++MaterialIndex)
							{
								if (MaterialSlots[MaterialSlotIndex].MaterialName == MaterialNames[MaterialIndex])
								{
									SelectedMaterialIdxAt[MaterialSlotIndex] = MaterialIndex;
								}
							}
						}

						// Material 선택
						for (uint64 MaterialSlotIndex = 0; MaterialSlotIndex < MeshGroupCount; ++MaterialSlotIndex)
						{
							ImGui::PushID(static_cast<int>(MaterialSlotIndex));
							if (ImGui::Combo("Material", &SelectedMaterialIdxAt[MaterialSlotIndex], MaterialNamesCharP.data(), static_cast<int>(MaterialNamesCharP.size())))
							{
								SMC->SetMaterialByUser(static_cast<uint32>(MaterialSlotIndex), MaterialNames[SelectedMaterialIdxAt[MaterialSlotIndex]]);
							}
							ImGui::PopID();
						}
					}
			}
		}
			// Billboard Component가 선택된 경우 Sprite UI
			else if (UBillboardComponent* BBC = Cast<UBillboardComponent>(SelectedComponent))
			{
				ImGui::Separator();
				ImGui::Text("Billboard Component Settings");
				
				// Sprite 텍스처 경로 표시 및 변경
				FString CurrentTexture = BBC->GetTexturePath();
				ImGui::Text("Current Sprite: %s", CurrentTexture.c_str());
				
				// Editor/Icon 폴더에서 동적으로 스프라이트 옵션 로드
				static TArray<FString> SpriteOptions;
				static bool bSpriteOptionsLoaded = false;
				static int currentSpriteIndex = 0; // 현재 선택된 스프라이트 인덱스
				
				if (!bSpriteOptionsLoaded)
				{
					// Editor/Icon 폴더에서 .dds 파일들을 찾아서 추가
					SpriteOptions = GetIconFiles();
					bSpriteOptionsLoaded = true;
					
					// 현재 텍스처와 일치하는 인덱스 찾기
					FString currentTexturePath = BBC->GetTexturePath();
					for (int i = 0; i < SpriteOptions.size(); ++i)
					{
						if (SpriteOptions[i] == currentTexturePath)
						{
							currentSpriteIndex = i;
							break;
						}
					}
				}
				
				// 스프라이트 선택 드롭다운 메뉴
				ImGui::Text("Sprite Texture:");
				FString currentDisplayName = (currentSpriteIndex >= 0 && currentSpriteIndex < SpriteOptions.size()) 
					? GetBaseNameNoExt(SpriteOptions[currentSpriteIndex]) 
					: "Select Sprite";
				
				if (ImGui::BeginCombo("##SpriteCombo", currentDisplayName.c_str()))
				{
					for (int i = 0; i < SpriteOptions.size(); ++i)
					{
						FString displayName = GetBaseNameNoExt(SpriteOptions[i]);
						bool isSelected = (currentSpriteIndex == i);
						
						if (ImGui::Selectable(displayName.c_str(), isSelected))
						{
							currentSpriteIndex = i;
							BBC->SetTexture(SpriteOptions[i]);
						}
						
						// 현재 선택된 항목에 포커스 설정
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				
				// 새로고침 버튼 (같은 줄에)
				ImGui::SameLine();
				if (ImGui::Button("Refresh"))
				{
					bSpriteOptionsLoaded = false; // 다음에 다시 로드하도록
					currentSpriteIndex = 0; // 인덱스 리셋
				}
				
				ImGui::Spacing();
				
				
				// Screen Size Scaled 체크박스
				// bool bIsScreenSizeScaled = BBC->IsScreenSizeScaled();
				// if (ImGui::Checkbox("Is Screen Size Scaled", &bIsScreenSizeScaled))
				// {
				// 	BBC->SetScreenSizeScaled(bIsScreenSizeScaled);
				// }
				
				// Screen Size (Is Screen Size Scaled가 true일 때만 활성화)
				if (false) // (bIsScreenSizeScaled)
				{
					float screenSize = BBC->GetScreenSize();
					if (ImGui::DragFloat("Screen Size", &screenSize, 0.0001f, 0.0001f, 0.1f, "%.4f"))
					{
						BBC->SetScreenSize(screenSize);
					}
				}
				else
				{
					// Billboard Size (Is Screen Size Scaled가 false일 때)
					float billboardWidth = BBC->GetBillboardWidth();
					float billboardHeight = BBC->GetBillboardHeight();
					
					if (ImGui::DragFloat("Width", &billboardWidth, 0.1f, 0.1f, 100.0f))
					{
						BBC->SetBillboardSize(billboardWidth, billboardHeight);
					}
					
					if (ImGui::DragFloat("Height", &billboardHeight, 0.1f, 0.1f, 100.0f))
					{
						BBC->SetBillboardSize(billboardWidth, billboardHeight);
					}
				}
				
				ImGui::Spacing();
				
				// UV 좌표 설정
				ImGui::Text("UV Coordinates");
				
				float u = BBC->GetU();
				float v = BBC->GetV();
				float ul = BBC->GetUL();
				float vl = BBC->GetVL();
				
				bool uvChanged = false;
				
				if (ImGui::DragFloat("U", &u, 0.01f))
					uvChanged = true;
					
				if (ImGui::DragFloat("V", &v, 0.01f))
					uvChanged = true;
					
				if (ImGui::DragFloat("UL", &ul, 0.01f))
					uvChanged = true;
					
				if (ImGui::DragFloat("VL", &vl, 0.01f))
					uvChanged = true;
				
				if (uvChanged)
				{
					BBC->SetUVCoords(u, v, ul, vl);
				}
			}
			else if (UTextRenderComponent* TextRenderComponent = Cast<UTextRenderComponent>(SelectedComponent))
			{
				ImGui::Separator();
				ImGui::Text("TextRender Component Settings");

				static char textBuffer[256];
				static UTextRenderComponent* lastSelected = nullptr;
				if (lastSelected != TextRenderComponent)
				{
					strncpy_s(textBuffer, sizeof(textBuffer), TextRenderComponent->GetText().c_str(), sizeof(textBuffer) - 1);
					lastSelected = TextRenderComponent;
				}

				ImGui::Text("Text Content");

				if (ImGui::InputText("##TextContent", textBuffer, sizeof(textBuffer)))
				{
					// 실시간으로 SetText 함수 호출
					TextRenderComponent->SetText(FString(textBuffer));
				}

				ImGui::Spacing();

				//// 4. 텍스트 색상을 편집하는 Color Picker를 추가합니다.
				//FLinearColor currentColor = TextRenderComponent->GetTextColor();
				//float color[3] = { currentColor.R, currentColor.G, currentColor.B }; // ImGui는 float 배열 사용

				//ImGui::Text("Text Color");
				//if (ImGui::ColorEdit3("##TextColor", color))
				//{
				//	// 색상이 변경되면 컴포넌트의 SetTextColor 함수를 호출
				//	TextRenderComponent->SetTextColor(FLinearColor(color[0], color[1], color[2]));
				//}
			}
			else if (UDecalComponent* DecalComponent = Cast<UDecalComponent>(SelectedComponent))
			{
				ImGui::Separator();
				ImGui::Text("Decal Component Settings");

				// 현재 선택된 데칼 경로 표시
				FString DecalFilePath = DecalComponent->GetTexturePath();
				// UTexture* DecalTexture = DecalComponent->GetMaterial()->GetTexture();
				// FString DecalFilePath{};				
				// if (DecalTexture)
				// {
				// 	DecalFilePath = DecalTexture->GetFilePath();
				// }
				ImGui::Text("Current Decal: %s", DecalFilePath.c_str());

				// Edidtor/Decal 폴더의 텍스처 로드
				static TArray<FString> DecalOptions;
				static bool bIsDecalOptionsLoaded = false;
				static int32 CurrentDecalIndex = 0;

				if (!bIsDecalOptionsLoaded)
				{
					DecalOptions = GetDecalFiles();
					bIsDecalOptionsLoaded = true;

					for (size_t i = 0; i< DecalOptions.size(); i++)
					{
						if (DecalOptions[i] == DecalFilePath)
						{
							CurrentDecalIndex = i;
							break;
						}
					}
				}

				// 데칼 드랍다운 메뉴
				ImGui::Text("Decal Texture: ");
				FString CurrentDisplayName = (CurrentDecalIndex >= 0 && CurrentDecalIndex < DecalOptions.size())
				? GetBaseNameNoExt(DecalOptions[CurrentDecalIndex]) : "Select Decal";

				if (ImGui::BeginCombo("##DecalCombo", CurrentDisplayName.c_str()))
				{
					for (int32 i = 0; i< DecalOptions.size(); i++)
					{
						FString DisplayName = GetBaseNameNoExt(DecalOptions[i]);
						bool bIsSelected = (CurrentDecalIndex == i);

						if (ImGui::Selectable(DisplayName.c_str(), bIsSelected))
						{
							CurrentDecalIndex = i;
							DecalComponent->SetTexture(DecalOptions[CurrentDecalIndex]);
						}

						if (bIsSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				ImGui::SameLine();
				if(ImGui::Button("Refresh"))
				{
					bIsDecalOptionsLoaded = false;
					CurrentDecalIndex = 0;
				}
				ImGui::Spacing();

				ImGui::Text("Decal Fade");
				ImGui::SameLine();				
				static TArray<FString> FadeTypes = {"In", "Out", "Loop"};
				static int32 CurrentFadeIndex = 0;

				FFadeProperty &FadeProperties = DecalComponent->GetFadeProperties();

				ImGui::Checkbox("Fade On", &FadeProperties.bIsFadeEnabled);
				if (FadeProperties.bIsFadeEnabled)
				{
					ImGui::SameLine();
					if (ImGui::Button("Start"))
					{
						FadeProperties.bIsFadeStart = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("Stop"))
					{
						FadeProperties.bIsFadeStart = false;
					}
					ImGui::SameLine();
					// 기본값으로 초기화
					if (ImGui::Button("Reset"))
					{
						DecalComponent->ResetFadeProperties();						
					}
					if (ImGui::BeginCombo("##FadeType", FadeTypes[CurrentFadeIndex].c_str()))
					{
						for (int32 i = 0; i< FadeTypes.size(); i++)
						{
							bool bIsSelected = (CurrentFadeIndex == i);
							if (ImGui::Selectable(FadeTypes[i].c_str(), bIsSelected))
							{
								CurrentFadeIndex = i;								
							}

							if (bIsSelected)
							{
								ImGui::SetItemDefaultFocus();
							}
						}
						ImGui::EndCombo();
					}
					ImGui::DragFloat("Duration", &FadeProperties.AlphaProperties.X, 0.01f, 0.0f, 60.0f);
					ImGui::DragFloat("MinAlpha", &FadeProperties.AlphaProperties.Y, 0.01f, 0.0f, FadeProperties.AlphaProperties.Z);
					ImGui::DragFloat("MaxAlpha", &FadeProperties.AlphaProperties.Z, 0.01f, FadeProperties.AlphaProperties.Y, 1.0f);
					switch (CurrentFadeIndex)
					{
					case 0:
						FadeProperties.Type = EFadeTypes::FadeIn;
						break;
					case 1:
						FadeProperties.Type = EFadeTypes::FadeOut;
						break;
					case 2:
						{
							//FadeProperties.Type = EFadeTypes::FadeLoop;
							FadeProperties.bIsLoop = true;
							break;
						}
					default:
						FadeProperties.Type = EFadeTypes::FadeIn;
						break;						
					}

					

				}
				
			}
			else if (USpotlightComponent* SpotlightComponent = Cast<USpotlightComponent>(SelectedComponent))
			{
				ImGui::Separator();
				ImGui::Text("Spotlight Component Settings");

				float coneAngle = SpotlightComponent->GetConeAngle();
				if (ImGui::DragFloat("Cone Angle", &coneAngle, 1.0f, 1.0f, 179.0f))
				{
					SpotlightComponent->SetConeAngle(coneAngle);
				}

				float attenuationRadius = SpotlightComponent->GetAttenuationRadius();
				if (ImGui::DragFloat("Attenuation Radius", &attenuationRadius, 1.0f, 0.1f, 1000.0f))
				{
					SpotlightComponent->SetAttenuationRadius(attenuationRadius);
				}
			}
			else if (URotationMovementComponent* RotationMovementComponent = Cast<URotationMovementComponent>(SelectedComponent))
			{
				ImGui::Text("RotationMovementComponent Settings");

				FVector RotationAngle = RotationMovementComponent->GetRotationAngle();

				// Location 편집
				if (ImGui::DragFloat3("RotationAngle", &RotationAngle.X, 0.1f)) {}

				RotationMovementComponent->SetRotationAngle(RotationAngle);
			}
			else if (UProjectileMovementComponent* ProjectileMovementComponent = Cast<UProjectileMovementComponent>(SelectedComponent))
			{
				ImGui::Text("RotationMovementComponent Settings");

				FVector LaunchDirection = ProjectileMovementComponent->GetLaunchDirection();

				// Location 편집
				if (ImGui::DragFloat3("Launch Direction", &LaunchDirection.X, 0.1f)) {}

				ProjectileMovementComponent->SetLaunchDirection(LaunchDirection);
			}
			else if (UHeightFogComponent* HeightFogComponent = Cast<UHeightFogComponent>(SelectedComponent))
			{
				ImGui::Separator();
				ImGui::Text("HeightFog Component Settings");

				// Fog Density
				float FogDensity = HeightFogComponent->GetFogDensity();
				if (ImGui::DragFloat("Fog Density", &FogDensity, 0.01f, 0.0f, 10.0f))
				{
					HeightFogComponent->SetFogDensity(FogDensity);
				}

				// Fog Height Falloff
				float FogHeightFalloff = HeightFogComponent->GetFogHeightFalloff();
				if (ImGui::DragFloat("Fog Height Falloff", &FogHeightFalloff, 0.01f, 0.0f, 10.0f))
				{
					HeightFogComponent->SetFogHeightFalloff(FogHeightFalloff);
				}

				// Start Distance
				float StartDistance = HeightFogComponent->GetStartDistance();
				if (ImGui::DragFloat("Start Distance", &StartDistance, 1.0f, 0.0f, 10000.0f))
				{
					HeightFogComponent->SetStartDistance(StartDistance);
				}

				// Fog Cutoff Distance
				float FogCutoffDistance = HeightFogComponent->GetFogCutoffDistance();
				if (ImGui::DragFloat("Fog Cutoff Distance", &FogCutoffDistance, 10.0f, 0.0f, 10000.0f))
				{
					HeightFogComponent->SetFogCutoffDistance(FogCutoffDistance);
				}

				// Fog Max Opacity
				float FogMaxOpacity = HeightFogComponent->GetFogMaxOpacity();
				if (ImGui::DragFloat("Fog Max Opacity", &FogMaxOpacity, 0.01f, 0.0f, 1.0f))
				{
					HeightFogComponent->SetFogMaxOpacity(FogMaxOpacity);
				}

				ImGui::Spacing();

				// Fog Inscattering Color
				FLinearColor FogColor = HeightFogComponent->GetFogInscatteringColor();
				float color[4] = { FogColor.R, FogColor.G, FogColor.B, FogColor.A };

				ImGui::Text("Fog Inscattering Color");
				if (ImGui::ColorEdit4("##FogColor", color))
				{
					HeightFogComponent->SetFogInscatteringColor(FLinearColor(color[0], color[1], color[2], color[3]));
				}
			}
			else if (UFireBallComponent* FireBallComponent = Cast<UFireBallComponent>(SelectedComponent))
			{
				ImGui::Separator();
				ImGui::Text("FireBall Component Settings");

				// Intensity
				float Intensity = FireBallComponent->GetIntensity();
				if (ImGui::DragFloat("Intensity", &Intensity, 0.1f, 0.0f, 100.0f))
				{
					FireBallComponent->SetIntensity(Intensity);
				}

				// Radius
				float Radius = FireBallComponent->GetRadius();
				if (ImGui::DragFloat("Radius", &Radius, 0.1f, 0.1f, 1000.0f))
				{
					FireBallComponent->SetRadius(Radius);
				}

				// Falloff
				float Falloff = FireBallComponent->GetRadiusFallOff();
				if (ImGui::DragFloat("Falloff", &Falloff, 0.1f, 0.1f, 10.0f))
				{
					FireBallComponent->SetRadiusFallOff(Falloff);
				}

				ImGui::Spacing();

				// Color
				FLinearColor FireballColor = FireBallComponent->GetColor();
				float color[4] = { FireballColor.R, FireballColor.G, FireballColor.B, FireballColor.A };

				ImGui::Text("Color");
				if (ImGui::ColorEdit4("##FireBallColor", color))
				{
					FireBallComponent->SetColor(FLinearColor(color[0], color[1], color[2], color[3]));
				}
			}
			else
			{
				ImGui::Text("Selected component is not a supported type.");
			}
		}
		
	}
	else
	{
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No Actor Selected");
		ImGui::TextUnformatted("Select an actor to edit its transform.");
	}

	ImGui::Separator();
}

// 재귀적으로 모든 하위 컴포넌트를 트리 형태로 렌더링
void UTargetActorTransformWidget::RenderComponentHierarchy(USceneComponent* SceneComponent)
{
	if (!SceneComponent)
	{
		return;
	}

	if (!SelectedActor || !SelectedComponent)
	{
		return;
	}

	const bool bIsRootComponent = SelectedActor->GetRootComponent() == SceneComponent;
	const FString ComponentName = SceneComponent->GetName().ToString() + (bIsRootComponent ? " (Root)" : "");
	const TArray<USceneComponent*>& AttachedChildren = SceneComponent->GetAttachChildren();
	const bool bHasChildren = AttachedChildren.Num() > 0;

	ImGuiTreeNodeFlags NodeFlags = ImGuiTreeNodeFlags_OpenOnArrow
		| ImGuiTreeNodeFlags_SpanAvailWidth
		| ImGuiTreeNodeFlags_DefaultOpen;

	// 현재 그리고 있는 SceneComponent가 SelectedComponent와 일치하는지 확인
	const bool bIsSelected = (SelectedComponent == SceneComponent);
	if (bIsSelected)
	{
		// 일치하면 Selected 플래그를 추가하여 하이라이트 효과를 줍니다.
		NodeFlags |= ImGuiTreeNodeFlags_Selected;
	}
	if (!bHasChildren)
	{
		NodeFlags |= ImGuiTreeNodeFlags_Leaf;
	}

	const bool bNodeIsOpen = ImGui::TreeNodeEx(
		(void*)SceneComponent,
		NodeFlags,
		"%s",
		ComponentName.c_str()
	);

	// 방금 그린 TreeNode가 클릭되었는지 확인합니다.
	if (ImGui::IsItemClicked())
	{
		// 클릭되었다면, 멤버 변수인 SelectedComponent를 현재 컴포넌트로 업데이트합니다.
		SelectedComponent = SceneComponent;
	}

	if (bNodeIsOpen)
	{
		for (USceneComponent* ChildComponent : AttachedChildren)
		{
			RenderComponentHierarchy(ChildComponent);
		}
		ImGui::TreePop();
	}
}

void UTargetActorTransformWidget::PostProcess()
{
	// 자동 적용이 활성화된 경우 변경사항을 즉시 적용
	if (bPositionChanged || bRotationChanged || bScaleChanged)
	{
		ApplyTransformToActor();
		ResetChangeFlags(); // 적용 후 플래그 리셋
	}
}

void UTargetActorTransformWidget::UpdateTransformFromActor()
{
	if (!SelectedActor || !SelectedComponent)
		return;	
	
	USceneComponent* SceneComponent = Cast<USceneComponent>(SelectedComponent);
	if (!SceneComponent)
		return;

	// 액터의 현재 트랜스폼을 UI 변수로 복사
	EditLocation = SceneComponent->GetRelativeLocation();
	EditRotation = SceneComponent->GetRelativeRotation().ToEuler();
	EditScale = SceneComponent->GetRelativeScale();

	ResetChangeFlags();
}

void UTargetActorTransformWidget::ApplyTransformToActor() const
{
	if (!SelectedActor || !SelectedComponent)
		return;

	bool bTransformHasChanged = false;

	USceneComponent* SceneComponent = Cast<USceneComponent>(SelectedComponent);
	if (!SceneComponent)
		return;

	// 변경사항이 있는 경우에만 적용
	if (bPositionChanged)
	{
		SceneComponent->SetRelativeLocation(EditLocation);
		bTransformHasChanged = true;
		UE_LOG("Transform: Applied location (%.2f, %.2f, %.2f)",
			EditLocation.X, EditLocation.Y, EditLocation.Z);
	}

	if (bRotationChanged)
	{
		FQuat NewRotation = FQuat::MakeFromEuler(EditRotation);
		SceneComponent->SetRelativeRotation(NewRotation);
		bTransformHasChanged = true;
		UE_LOG("Transform: Applied rotation (%.1f, %.1f, %.1f)",
			EditRotation.X, EditRotation.Y, EditRotation.Z);
	}

	if (bScaleChanged)
	{
		SceneComponent->SetRelativeScale(EditScale);
		bTransformHasChanged = true;
		UE_LOG("Transform: Applied scale (%.2f, %.2f, %.2f)",
			EditScale.X, EditScale.Y, EditScale.Z);
	}

	// 컴포넌트의 트랜스폼이 변경되었다면, World에 알려 BVH를 업데이트 하도록 합니다.
	if (bTransformHasChanged)
	{
		if (UWorld* World = SelectedActor->GetWorld())
		{
			World->MarkBVHDirty();
		}
	}

	// 플래그 리셋은 const 메서드에서 할 수 없으므로 PostProcess에서 처리
}

void UTargetActorTransformWidget::ResetChangeFlags()
{
	bPositionChanged = false;
	bRotationChanged = false;
	bScaleChanged = false;
}
