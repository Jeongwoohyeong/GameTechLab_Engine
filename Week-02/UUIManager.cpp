#include <Windows.h>
#include "UUIManager.h"
#include "FTransform.h"
#include "UCamera.h"
#include "CScene.h"
#include "UObject.h"

void UUIManager::Initialize(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(device, deviceContext);

	// bShowConsoleWindow = true;
	UE_LOG("UI Manager Initialized");
}

void UUIManager::PrepareRender()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void UUIManager::Render()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void UUIManager::ControlPanel()
{
	ImGui::Begin("Jungle Control Panel");
	{
		{
			// FPS 표시
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
			// 한 프레임당 걸린 시간 표시 ms
			ImGui::Text("(%.3f ms)", 1000.0f / ImGui::GetIO().Framerate);
		}
		ImGui::Separator();
		{
			// Primitive type 목록 리스트 중 목록 선택 가능
			// 목록 리스트 토글 가능 (열고/닫기)
			static int CurrentType = 0;
			static const char* Types[] = { "Cube", "Sphere", "Triangle"};
			ImGui::Combo("Primitive Type", &CurrentType, Types, IM_ARRAYSIZE(Types));

			// SCene에 CurrenyType에 해당하는 프리미티브 추가
			// Number of Spawn 설정에 따라 여러 개 추가
			static int NumberOfSpawn = 1;
			if (ImGui::Button("Spawn"))
			{
				CScene::GetInstance().Spawn((EPrimitiveType)CurrentType, NumberOfSpawn);
			}
			ImGui::SameLine();
			if (ImGui::DragInt("Number of Spawn", &NumberOfSpawn, 1.0f, 1, 100))
			{
				if (NumberOfSpawn < 1) 
				{
					NumberOfSpawn = 1;
				}
				if (NumberOfSpawn > 100) 
				{
					NumberOfSpawn = 100;
				}
			}
	
		}
		ImGui::Separator();
		{
			// Scene
			CScene& scene = CScene::GetInstance();

			// Scene Name 입력
			static char sceneName[128] = "Default";
			ImGui::InputText("Scene Name", sceneName, IM_ARRAYSIZE(sceneName));

			// New Scene
			if (ImGui::Button("New Scene"))
			{
				scene.New();
			}
			// Save Scene
			if (ImGui::Button("Save Scene"))
			{
				scene.Save(sceneName);
			}
			// Load Scene
			if (ImGui::Button("Load Scene"))
			{
				scene.Load(sceneName);
			}
		}
		ImGui::Separator();
		{
			ImGui::Checkbox("Orthogonal", &UCamera::GetInstance().bIsOrthogonal);
			// 카메라 위치 조정 (float3)
			FVector Location = SwapYZ(UCamera::GetInstance().Location);
			if (ImGui::DragFloat3("Camera Location", &Location.X, 0.01f, -100.0f, 100.0f))
			{
				UCamera::GetInstance().Location = SwapYZ(Location);
			}

			// 카메라 회전 조정 (오일러 값, float3)
			FVector Rotation = UCamera::GetInstance().Rotation * Math::RadToDeg;
			Rotation = SwapYZ(Rotation); // UI용으로 YZ 스왑
			if (ImGui::DragFloat3("Camera Rotation", &Rotation.X, 0.1f, -360.0f, 360.0f))
			{
				UCamera::GetInstance().Rotation = SwapYZ(Rotation) * Math::DegToRad;
			}

			// 카메라 FovY, Near, Far 조정
			float fovYdeg = UCamera::GetInstance().FovY * Math::RadToDeg;
			if (ImGui::SliderFloat("Fov Y", &fovYdeg, 1.0f, 120.0f, "%.1f"))
			{
				UCamera::GetInstance().FovY = fovYdeg * Math::DegToRad;
			}

			float zn = UCamera::GetInstance().NearPlane;
			if (ImGui::DragFloat("Near", &zn, 0.01f, 0.01f, 10.0f, "%.3f"))
			{
				UCamera::GetInstance().NearPlane = zn;
			}

			float zf = UCamera::GetInstance().FarPlane;
			if (ImGui::DragFloat("Far", &zf, 1.0f, 10.0f, 100000.0f, "%.1f"))
			{
				UCamera::GetInstance().FarPlane = zf;
			}
		}
		ImGui::Separator();
		{
			// SCene에 있는 모든 프리미티브의 UUID 콤보박스
			// UUID 선택 후 Select 버튼 클릭 시 해당 프리미티브 선택
			CScene& Scene = CScene::GetInstance();
			static int CurrentUUIDIndex = 0;
			static ImVector<uint32> UUIDList;
			UUIDList.clear();
			for (const auto& Pair : Scene.GetPrimitives())
			{
				UUIDList.push_back(Pair.first);
			}

			if (UUIDList.size() > 0)
			{
				// Index 안전 처리
				if (CurrentUUIDIndex < 0) 
				{
					CurrentUUIDIndex = 0;
				}
				if (CurrentUUIDIndex >= (int)UUIDList.size()) 
				{
					CurrentUUIDIndex = (int)UUIDList.size() - 1;
				}

				// 1) Select 버튼 먼저
				if (ImGui::Button("Select"))
				{
					Scene.SetSelectedPrimitiveByUUID(UUIDList[CurrentUUIDIndex]);
				}

				// 2) 같은 줄에 콤보박스
				ImGui::SameLine();
				ImGui::SetNextItemWidth(180.0f);
				if (ImGui::BeginCombo("UUID", std::to_string(UUIDList[CurrentUUIDIndex]).c_str()))
				{
					for (int n = 0; n < UUIDList.size(); ++n)
					{
						const bool bIsSelected = (CurrentUUIDIndex == n);
						if (ImGui::Selectable(std::to_string(UUIDList[n]).c_str(), bIsSelected))
						{
							CurrentUUIDIndex = n;
						}
						if (bIsSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				// 선택된 프리미티브 씬에서 제거
				if (ImGui::Button("Destroy Selected"))
				{
					Scene.DestroySelectedPrimitive();
				}

				// 오브젝트 메모리 통계
				ImGui::Text("Total Allocation Bytes: %llu", UObject::TotalAllocationBytes);
				ImGui::Text("Total Allocation Count: %llu", UObject::TotalAllocationCount);
			}
			else
			{
				ImGui::Text("No Primitives in Scene");
			}
		}
	}
	ImGui::End();
}

void UUIManager::ConsoleWindow(bool bShowConsoleWindow)
{
	ImGui::ShowExampleAppConsoleWindow(&bShowConsoleWindow);
}

void UUIManager::PropertyWindow(UPrimitiveComponent* Primitive)
{
	ImGui::Begin("Jungle Property Window");
	if (Primitive)
	{
		FTransform* Transform = Primitive->GetTransform();

		// 1) Scale
		FVector Scale = SwapYZ(Transform->GetScale()); // UI용으로 YZ 스왑
		if (ImGui::DragFloat3("Scale", &Scale.X, 0.01f, 0.01f, 100.0f)) {
			Transform->SetScale(SwapYZ(Scale));
		}
		if (ImGui::Button("S Reset")) {
			FVector s(1.0f, 1.0f, 1.0f);
			Transform->SetScale(s);
		}

		// 2) Rotation (deg)
		// 내부는 rad라고 가정 → UI용으로 deg로 변환해 표기
		FVector PrevRotation = Transform->GetRotationRadians();
		FVector CurRotation = SwapYZ(PrevRotation * Math::RadToDeg);
		if (ImGui::DragFloat3("Rotation", &CurRotation.X, 0.1f, -360.0f, 360.0f)) {
			Transform->SetRotationDeg(SwapYZ(CurRotation)); // SetRotationDeg는 내부에서 deg→rad 처리
			
			// 쿼터니언 업데이트
			FVector DeltaRotation = Transform->GetRotationRadians() - PrevRotation;
			Transform->UpdateQuaternion(DeltaRotation);
		}
		if (ImGui::Button("R Reset")) {
			FVector r0(0.0f, 0.0f, 0.0f);
			Transform->SetRotationDeg(r0);
		}

		// 3) Translation
		FVector Location = SwapYZ(Transform->GetLocation());
		if (ImGui::DragFloat3("Translation", &Location.X, 0.01f, -100.0f, 100.0f)) {
			Transform->SetLocation(SwapYZ(Location));
		}
		if (ImGui::Button("T Reset")) {
			Transform->SetLocation(FVector(0.0f, 0.0f, -10.0f));
		}

		if (ImGui::Button("Primitive Type Check"))
		{
			Primitive->IsA();
		}
	}
	ImGui::End();
}

void UUIManager::ReleaseUI()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
