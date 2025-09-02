#pragma once

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include <d3d11.h>
#include <d3dcompiler.h>

class D3DUtil
{
public:
	static void CreateVSAndInputLayout(LPCWSTR& fileName, ID3D11VertexShader* vs, ID3D11InputLayout* inputLayout);
	static void CreatePS(LPCWSTR& fileName, ID3D11PixelShader* ps);
};