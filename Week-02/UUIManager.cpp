#include <Windows.h>
#include "UUIManager.h"
#include "FTransform.h"
#include "UCamera.h"

void UUIManager::Initialize(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(device, deviceContext);

	bShowConsoleWindow = true;
	UE_LOG("UI Manager Initialized");
}

void UUIManager::ObjectControlUI(FTransform* object)
{
	ObjectTransform = object;
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	/***********************/
	// ImGui UI 추가 위치 - ImGui::NewFrame()과 ImGui::Render() 사이에 위치한다.
	ImGui::Begin("Jungle Property Window");
	{
		ObjectControl();
		ImGui::ShowExampleAppConsoleWindow(&bShowConsoleWindow);
	}
	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void UUIManager::ReleaseUI()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void UUIManager::ObjectControl()
{
	// 스케일 조정 (float3)
	static FVector scale = FVector(1.0f, 1.0f, 1.0f);
	ImGui::Text("Scale");
	ImGui::DragFloat3("##scale", &scale.X, 0.01f, 0.01f, 100.0f);
	if (ImGui::Button("S Reset"))
	{
		scale = FVector(1.0f, 1.0f, 1.0f);
	};
	ObjectTransform->SetScale(scale);

	// 회전 조정 (오일러 값, float3)
	static FVector rotationDeg = FVector(0.0f, 0.0f, 0.0f);
	ImGui::Text("Rotation(Deg)");
	ImGui::DragFloat3("rotation (Deg)", &rotationDeg.X, 0.1f, -360.0f, 360.0f);
	if (ImGui::Button("R Reset"))
	{
		rotationDeg = FVector(0.0f, 0.0f, 0.0f);
	};
	ObjectTransform->SetRotation(rotationDeg);

	// 이동 조정 (float3)
	static FVector translation = FVector(0.0f, 0.0f, 0.0f);
	ImGui::Text("Translation");
	ImGui::DragFloat3("##translation", &translation.X, 0.01f, -100.0f, 100.0f);
	if (ImGui::Button("T Reset"))
	{
		translation = FVector(0.0f, 0.0f, -10.0f);
	};
	ObjectTransform->SetLocation(translation);

	// 카메라 위치 조정
	ImGui::Text("CameraPos");
	ImGui::DragFloat3("##CameraPos", &(UCamera::GetInstance().Location.X), 0.01f, -100.0f, 100.0f);

	// 카메라 회전 조정 (오일러 값, float3)
	ImGui::Text("CameraRot(Deg)");
	static FVector CameraRotDeg = FVector(0.0f, 0.0f, 0.0f);
	ImGui::DragFloat3("##CameraRot", &CameraRotDeg.X, 0.1f, -360.0f, 360.0f);
	UCamera::GetInstance().Rotation = FVector(
		Math::DegToRad * CameraRotDeg.X,
		Math::DegToRad * CameraRotDeg.Y,
		Math::DegToRad * CameraRotDeg.Z
	);

	// 카메라 FovY, Near, Far 조정
	ImGui::Text("Fov Y, Near, Far");
	float FovYDegree = Math::RadToDeg * UCamera::GetInstance().FovY;
	ImGui::SliderFloat("Fov Y (deg)", &FovYDegree, 1.0f, 120.0f, "%.1f");
	UCamera::GetInstance().FovY = Math::DegToRad * FovYDegree;
	ImGui::DragFloat("Near (zn)", &UCamera::GetInstance().NearPlane, 0.01f, 0.01f, 10.0f, "%.3f");
	ImGui::DragFloat("Far  (zf)", &UCamera::GetInstance().FarPlane, 1.0f, 10.0f, 100000.0f, "%.1f");
}
