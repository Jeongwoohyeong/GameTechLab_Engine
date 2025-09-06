#pragma once
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "FVector.h"

class FTransform;

class UUIManager
{
public:
	UUIManager() {};
	~UUIManager() {};

	void Initialize(HWND hWnd, ID3D11Device*, ID3D11DeviceContext*, FTransform*);
	void RenderUI();	
	void ReleaseUI();

private:
	void TransformUI();
	FTransform* Transform;
	FVector offset;
};
