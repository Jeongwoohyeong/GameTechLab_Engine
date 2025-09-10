#pragma once
#include <Windows.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_console.h"
#include "Math.h"

class FTransform;
class UPrimitiveComponent;

class UUIManager
{
public:
	UUIManager() {};
	~UUIManager() {};

	void Initialize(HWND hWnd, ID3D11Device*, ID3D11DeviceContext*);
	void ReleaseUI();

	void PrepareRender();
	void Render();

	void PropertyWindow(UPrimitiveComponent* Primitive);
	void ControlPanel();
	void ConsoleWindow(bool bShowConsoleWindow);

private:
	void DrawRotationInspector(FTransform& Transform);

private:
	FVector offset;

};
