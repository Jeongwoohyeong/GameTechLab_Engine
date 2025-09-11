#include <Windows.h>
#include "UUIManager.h"
#include "FTransform.h"
#include "UCamera.h"
#include "CScene.h"
#include "UObject.h"
#include "UCubeComp.h"
#include "USphereComp.h"
#include "UTriangleComp.h"

void UUIManager::Initialize(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(device, deviceContext);

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg].x = 0.0f; // 배경 색상 설정 (RGB)
	style.Colors[ImGuiCol_WindowBg].w = 0.7f; // 배경 투명

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
			static const char* Types[] = { "Cube", "Sphere", "Triangle", "Plane" };
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
			FVector Location = UCamera::GetInstance().Location;
			if (ImGui::DragFloat3("Camera Location", &Location.X, 0.01f, -100.0f, 100.0f))
			{
				UCamera::GetInstance().Location = Location;
			}

			// 카메라 회전 조정 (오일러 값, float3)
			FVector Rotation = UCamera::GetInstance().Rotation * Math::RadToDeg;
			if (ImGui::DragFloat3("Camera Rotation", &Rotation.X, 0.1f, -360.0f, 360.0f))
			{
				UCamera::GetInstance().Rotation = Rotation * Math::DegToRad;
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

void UUIManager::DrawRotationInspector(FTransform& TargetTransform)
{
	static FTransform* CurrentTarget = nullptr;

	// UI 표시에 쓰는 오일러 (Degree)
	static FVector EulerAnglesForUI;

	// 외부 변경 감지용 캐시 쿼터니언
	static FQuaternion CachedQuat;

	// 편집 세션 상태
	static bool        bUserEditing = false;
	static bool        bPrevActive = false;
	static FQuaternion StartQuat;        // 드래그(편집) 시작 시 원본 쿼터니언
	static FVector     StartEulerDeg;    // 드래그 시작 시 UI 오일러(deg)

	// 선택 변경 시 초기화
	if (&TargetTransform != CurrentTarget)
	{
		CurrentTarget = &TargetTransform;
		CachedQuat = TargetTransform.GetQuaternion();
		FVector eulerRad = FQuaternion::ToEulerAngles(CachedQuat);
		EulerAnglesForUI = RadToDeg(eulerRad);
		bUserEditing = false;
		bPrevActive = false;
		StartQuat = CachedQuat;
		StartEulerDeg = EulerAnglesForUI;
	}

	// q vs -q 동일성 처리 포함 근사 비교
	auto QuatNearlyEqual = [](const FQuaternion& A, const FQuaternion& B, float Eps = 1e-5f)
		{
			float dot = A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
			FQuaternion BAdj = (dot < 0.0f) ? FQuaternion{ -B.X, -B.Y, -B.Z, -B.W } : B;
			return (fabs(A.X - BAdj.X) <= Eps &&
				fabs(A.Y - BAdj.Y) <= Eps &&
				fabs(A.Z - BAdj.Z) <= Eps &&
				fabs(A.W - BAdj.W) <= Eps);
		};

	// 외부에서 회전 변경된 경우 (편집 중이 아닐 때만) UI 재동기화
	FQuaternion CurrentQuat = TargetTransform.GetQuaternion();
	if (!bUserEditing && !QuatNearlyEqual(CurrentQuat, CachedQuat))
	{
		CachedQuat = CurrentQuat;
		FVector eulerRad = FQuaternion::ToEulerAngles(CachedQuat);
		EulerAnglesForUI = RadToDeg(eulerRad);
		StartQuat = CachedQuat;
		StartEulerDeg = EulerAnglesForUI;
	}

	ImGui::PushID("RotationInspector");

	bool bValueChanged = ImGui::DragFloat3("Rotation", &EulerAnglesForUI.X, 0.1f);
	bool bActiveNow = ImGui::IsItemActive();

	// 편집 시작 감지 (이번 프레임 active && 이전 프레임 inactive)
	if (bActiveNow && !bPrevActive)
	{
		StartQuat = TargetTransform.GetQuaternion();
		StartEulerDeg = EulerAnglesForUI; // 현재 UI 값을 기준점으로
	}

	// Reset 버튼
	if (ImGui::Button("R Reset"))
	{
		TargetTransform.ClearRotation();
		CachedQuat = TargetTransform.GetQuaternion();
		FVector eulerRad = FQuaternion::ToEulerAngles(CachedQuat);
		EulerAnglesForUI = RadToDeg(eulerRad);
		StartQuat = CachedQuat;
		StartEulerDeg = EulerAnglesForUI;
		// 편집 중 아님으로 플래그 클리어
		bUserEditing = false;
		bPrevActive = false;
	}

	// 편집 중(Active)일 때는 델타 방식으로 매프레임 적용
	if (bActiveNow)
	{
		// ΔEuler = (현재 UI - 시작 UI)
		FVector DeltaDeg = EulerAnglesForUI - StartEulerDeg;

		// 각 축별 Wrap (-180~180)
		auto Wrap = [](float a)->float {
			while (a > 180.f) a -= 360.f;
			while (a < -180.f) a += 360.f;
			return a;
			};
		DeltaDeg.X = Wrap(DeltaDeg.X);
		DeltaDeg.Y = Wrap(DeltaDeg.Y);
		DeltaDeg.Z = Wrap(DeltaDeg.Z);

		// 델타 쿼터니언 생성 (CreateFromEulerAngles는 (Rad) 입력)
		FQuaternion DeltaQuat = FQuaternion::CreateFromEulerAngles(DegToRad(DeltaDeg));
		// 누적: 새 회전 = Delta * Start
		FQuaternion NewQuat = FQuaternion::Multiply(DeltaQuat, StartQuat);
		NewQuat.Normalize();

		TargetTransform.LoadQuaternion(NewQuat); // LoadQuaternion: Dirty 플래그 포함
		CachedQuat = NewQuat;
	}
	else if (bPrevActive && !bActiveNow)
	{
		// 편집 종료: 최종 쿼터니언 → 안정화 Euler 재계산
		CachedQuat = TargetTransform.GetQuaternion();
		// 오일러 재계산 시 오차 발생 -> 제외
		// FVector eulerRad = FQuaternion::ToEulerAngles(CachedQuat);
		// EulerAnglesForUI = RadToDeg(eulerRad);
		StartQuat = CachedQuat;
		StartEulerDeg = EulerAnglesForUI;
	}

	bUserEditing = bActiveNow;
	bPrevActive = bActiveNow;

	ImGui::PopID();
}

void UUIManager::PropertyWindow(UPrimitiveComponent* Primitive)
{
	ImGui::Begin("Jungle Property Window");
	if (Primitive)
	{
		FTransform* Transform = Primitive->GetTransform();
		bool bIsLocalMode = Primitive->GetGizmo()->bIsLocalMode;

		// 1) Scale
		FVector Scale = Transform->GetScale();
		if (ImGui::DragFloat3("Scale", &Scale.X, 0.01f, 0.01f, 100.0f)) {
			Transform->SetScale(Scale);
		}
		if (ImGui::Button("S Reset")) {
			FVector s(1.0f, 1.0f, 1.0f);
			Transform->SetScale(s);
		}

		// 2) Rotation
		DrawRotationInspector(*Transform);

		// 3) Translation
		FVector Location = Transform->GetLocation();
		if (ImGui::DragFloat3("Translation", &Location.X, 0.01f, -100.0f, 100.0f)) {
			Transform->SetLocation(Location);
		}
		if (ImGui::Button("T Reset")) {
			Transform->SetLocation(FVector(0.0f, 0.0f, -10.0f));
		}

		if (ImGui::Button("Primitive Type Check"))
		{
			if (Primitive->IsA(UCubeComp::StaticClass()))
			{
				UE_LOG("Cube");
			}

			if (Primitive->IsA(USphereComp::StaticClass()))
			{
				UE_LOG("Sphere");
			}

			if (Primitive->IsA(UTriangleComp::StaticClass()))
			{
				UE_LOG("Triangle");
			}
		}

		// 4) Gizmo Mode
		ImGui::Separator();
		{
			FLocalGizmo* Gizmo = Primitive->GetGizmo();
			bool bIsLocalMode = Gizmo->bIsLocalMode;
			if (ImGui::Checkbox("Local Gizmo", &bIsLocalMode))
			{
				Gizmo->bIsLocalMode = bIsLocalMode;
			}
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
