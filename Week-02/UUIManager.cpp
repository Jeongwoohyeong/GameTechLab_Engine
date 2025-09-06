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

	ObjectControl();

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
			// FVector를 float 배열로 변환
	static FVector scale = FVector(1.0f, 1.0f, 1.0f);
	ImGui::Text("Scale");
	ImGui::DragFloat3("##scale", &scale.X, 0.001f, 0.01f, 100.0f);
	if (ImGui::Button("S Reset"))
	{
		scale = FVector(1.0f, 1.0f, 1.0f);
	};
	ObjectTransform->SetScale(scale);

	// 회전 조정 (오일러 값, float3)
	static FVector rotationDeg = FVector(0.0f, 0.0f, 0.0f);
	ImGui::Text("Rotation(Deg)");
	ImGui::DragFloat3("rotation (Deg)", &rotationDeg.X, 1.0f, -360.0f, 360.0f);
	if (ImGui::Button("R Reset"))
	{
		rotationDeg = FVector(0.0f, 0.0f, 0.0f);
	};
	ObjectTransform->SetRotation(rotationDeg);

	// 이동 조정 (float3)
	static FVector translation = FVector(0.0f, 0.0f, 0.0f);
	ImGui::Text("Translation");
	ImGui::DragFloat3("##translation", &translation.X, 0.001f, -100.0f, 100.0f);
	if (ImGui::Button("T Reset"))
	{
		translation = FVector(0.0f, 0.0f, -10.0f);
	};
	ObjectTransform->SetLocation(translation);

	static FVector CameraPos = FVector(0.0f, 0.0f, 0.0f);
	ImGui::Text("CameraPos");
	ImGui::DragFloat3("##CameraPos", &CameraPos.X, 0.001f, -100.0f, 100.0f);
	//camera->SetPosition(CameraPos);
	UCamera::GetInstance().Location = CameraPos;

	static FVector CameraRot = FVector(0.0f, 0.0f, 0.0f);
	ImGui::Text("CameraRot(Deg)");
	ImGui::DragFloat3("##CameraRot", &CameraRot.X, 1.0f, -360.0f, 360.0f);
	//camera->SetRotation(CameraRot);

	static float fovY_deg = 60.0f;   // 일반적인 시작값
	static float zn = 0.1f;    // 너무 작으면 z-정밀도 손실, 너무 크면 가까운 게 잘림
	static float zf = 1000.0f; // 씬 규모에 맞게 조정
	ImGui::Text("Camera");
	ImGui::SliderFloat("FOV Y (deg)", &fovY_deg, 1.0f, 120.0f, "%.1f");   // 1~120도 권장
	ImGui::DragFloat("Near (zn)", &zn, 0.01f, 0.01f, 10.0f, "%.3f");      // 0.01~10
	ImGui::DragFloat("Far  (zf)", &zf, 1.0f, 10.0f, 100000.0f, "%.1f");   // 10~10만
	/*camera->SetFovY(fovY_deg);
	camera->SetZn(zn);
	camera->SetZf(zf);
	camera->SetAspect(GetAspectRatio(renderer));*/
}
