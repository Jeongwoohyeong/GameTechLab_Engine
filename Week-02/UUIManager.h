#pragma once
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_console.h"
#include "Math.h"

class FTransform;

class UUIManager
{
public:
	UUIManager() {};
	~UUIManager() {};

	void Initialize(HWND hWnd, ID3D11Device*, ID3D11DeviceContext*);
	void ObjectControlUI(FTransform* object);
	void ReleaseUI();

private:
	void ObjectControl();
	FTransform* ObjectTransform;
	FVector offset;
	bool bShowConsoleWindow;
};
