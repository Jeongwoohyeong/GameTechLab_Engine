#include <Windows.h>
#include "UUIManager.h"
#include "FTransform.h"
#include "UCamera.h"
#include "CScene.h"

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
	// 1) Scale
	ImGui::Text("Scale");
	FVector scale = ObjectTransform->GetScale();
	if (ImGui::DragFloat3("##scale", &scale.X, 0.01f, 0.01f, 100.0f)) {
		ObjectTransform->SetScale(scale);
	}
	if (ImGui::Button("S Reset")) {
		FVector s(1.0f, 1.0f, 1.0f);
		ObjectTransform->SetScale(s);
	}

	// 2) Rotation (deg)
	ImGui::Text("Rotation (Deg)");
	// 내부는 rad라고 가정 → UI용으로 deg로 변환해 표기
	FVector rotDeg = ObjectTransform->GetRotationRadians() * Math::RadToDeg;
	if (ImGui::DragFloat3("rotation (Deg)", &rotDeg.X, 0.1f, -360.0f, 360.0f)) {
		ObjectTransform->SetRotationDeg(rotDeg); // SetRotationDeg는 내부에서 deg→rad 처리
	}
	if (ImGui::Button("R Reset")) {
		FVector r0(0.0f, 0.0f, 0.0f);
		ObjectTransform->SetRotationDeg(r0);
	}

	// 3) Translation
	ImGui::Text("Translation");
	FVector pos = ObjectTransform->GetLocation();
	if (ImGui::DragFloat3("##translation", &pos.X, 0.01f, -100.0f, 100.0f)) {
		ObjectTransform->SetLocation(pos);
	}
	if (ImGui::Button("T Reset")) {
		ObjectTransform->SetLocation(FVector(0.0f, 0.0f, -10.0f));
	}
	// 카메라 위치 조정
	ImGui::Text("CameraPos");
	FVector cPos = UCamera::GetInstance().Location;
	if (ImGui::DragFloat3("##CameraPos", &cPos.X, 0.01f, -100.0f, 100.0f))
		UCamera::GetInstance().Location = cPos;

	// 카메라 회전 조정 (오일러 값, float3)
	ImGui::Text("CameraRot(Deg)");
	static FVector CameraRotDeg = FVector(0.0f, 0.0f, 0.0f);
	if (ImGui::DragFloat3("##CameraRot", &CameraRotDeg.X, 0.1f, -360.0f, 360.0f))
	{
		UCamera::GetInstance().Rotation = FVector(
			Math::DegToRad * CameraRotDeg.X,
			Math::DegToRad * CameraRotDeg.Y,
			Math::DegToRad * CameraRotDeg.Z
		);
	}

	// 카메라 FovY, Near, Far 조정
	float fovYdeg = UCamera::GetInstance().FovY * Math::RadToDeg;
	if (ImGui::SliderFloat("Fov Y (deg)", &fovYdeg, 1.0f, 120.0f, "%.1f"))
		UCamera::GetInstance().FovY = fovYdeg * Math::DegToRad;

	float zn = UCamera::GetInstance().NearPlane;
	if (ImGui::DragFloat("Near (zn)", &zn, 0.01f, 0.01f, 10.0f, "%.3f"))
		UCamera::GetInstance().NearPlane = zn;

	float zf = UCamera::GetInstance().FarPlane;
	if (ImGui::DragFloat("Far  (zf)", &zf, 1.0f, 10.0f, 100000.0f, "%.1f"))
		UCamera::GetInstance().FarPlane = zf;

	ImGui::Separator();

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
