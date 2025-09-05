#pragma once
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

class UUIManager
{
public:
	UUIManager() {};
	~UUIManager() {};

	void Initialize(HWND hWnd, ID3D11Device*, ID3D11DeviceContext*);
	void RenderUI();	
	void ReleaseUI();

private:
	void SomeUI();

};
