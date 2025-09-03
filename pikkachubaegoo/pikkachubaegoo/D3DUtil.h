#pragma once

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include <d3d11.h>
#include <d3dcompiler.h>
#include "DirectXTK/Inc/DDSTextureLoader.h"
#include "DirectXTK/Inc/WICTextureLoader.h"
using namespace DirectX;

class D3DUtil
{
public:
	static void CreateVSAndInputLayout(LPCWSTR& fileName, ID3D11VertexShader** vs, ID3D11InputLayout** inputLayout);
	static void CreatePS(LPCWSTR& fileName, ID3D11PixelShader** ps);
	static void CreateConstantBuffer(ID3D11Buffer** cBuffer, UINT size);
	static void LoadTexture(LPCWSTR& filePath, ID3D11ShaderResourceView** textureSRV);
	static void CreateSamplerState(ID3D11SamplerState** samplerState, D3D11_SAMPLER_DESC& desc);
	template<typename T>
	static void UpdateConstantBuffer(ID3D11DeviceContext* context, ID3D11Buffer* cBuffer, T& cBufferData)
	{
		D3D11_MAPPED_SUBRESOURCE constantbufferMSR;
		context->Map(cBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
		memcpy(constantbufferMSR.pData, &cBufferData, sizeof(T));

		context->Unmap(cBuffer, 0);
	}
};