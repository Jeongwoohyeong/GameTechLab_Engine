#include <Windows.h>
#include "UUIManager.h"
#include "FTransform.h"

void UUIManager::Initialize(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext, FTransform* transform)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(device, deviceContext);
	Transform = transform;
}

void UUIManager::RenderUI()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	/***********************/
	// ImGui UI 추가 위치 - ImGui::NewFrame()과 ImGui::Render() 사이에 위치한다.
	ImGui::Begin("Jungle Property Window");

	this->TransformUI();

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

void UUIManager::TransformUI()
{	
	ImGui::Text("Hello Jungle!");
	ImGui::SliderFloat("Translation", &offset.x, -1.0f, 1.0f);
	ImGui::SliderFloat("Rotation", &offset.y, -1.0f, 1.0f);
	ImGui::SliderFloat("TransScalelation", &offset.z, -1.0f, 1.0f);
	Transform->Translate(offset);	
}