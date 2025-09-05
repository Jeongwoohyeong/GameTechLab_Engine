#include <d3d11.h>
#include <d3dcompiler.h>
#include "UShader.h"

bool UShader::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	Device = device;
	DeviceContext = deviceContext;

	this->CreateVertexShader();
	this->CreatePixelShader();
	this->CreateConstBuffer();

	return true;
}

void UShader::PrepareShader()
{
	DeviceContext->IASetInputLayout(InputLayout);
	DeviceContext->VSSetShader(VertexShader, nullptr, 0);
	DeviceContext->PSSetShader(PixelShader, nullptr, 0);

	if (ConstantBuffer)
	{
		DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
	}
}

void UShader::UpdateContant(float x, float y, float z, float scale)
{
	if (!ConstantBuffer)
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE constantBufferMSR;

	DeviceContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantBufferMSR);
	FConstants* constants = (FConstants*)constantBufferMSR.pData;
	constants->offset = { x, y, z };
	constants->scale = 0.05f;
	DeviceContext->Unmap(ConstantBuffer, 0);
}

void UShader::Release()
{
	if (ConstantBuffer)
	{
		ConstantBuffer->Release();
		ConstantBuffer = nullptr;
	}

	if (PixelShader)
	{
		PixelShader->Release();
		PixelShader = nullptr;
	}

	if (InputLayout)
	{
		InputLayout->Release();
	}

	if (VertexShader)
	{
		VertexShader->Release();
		VertexShader = nullptr;
	}
}

bool UShader::CreateVertexShader()
{
	HRESULT result;
	ID3DBlob* vertexShaderCSO = nullptr;
	ID3DBlob* errorMessage = nullptr;	
	
	result = D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &vertexShaderCSO, nullptr);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			MessageBoxA(nullptr, (char*)errorMessage->GetBufferPointer(), "VS compile error", MB_OK);
		}
		return false;
	}

	Device->CreateVertexShader(vertexShaderCSO->GetBufferPointer(), 
		vertexShaderCSO->GetBufferSize(), nullptr, &VertexShader);
	
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	Device->CreateInputLayout(layout, ARRAYSIZE(layout), vertexShaderCSO->GetBufferPointer(),
		vertexShaderCSO->GetBufferSize(), &InputLayout);

	vertexShaderCSO->Release();

	return true;
}

bool UShader::CreatePixelShader()
{
	HRESULT result;
	ID3DBlob* pixelShaderCSO = nullptr;
	ID3DBlob* errorMessage = nullptr;

	result = D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &pixelShaderCSO, nullptr);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			MessageBoxA(nullptr, (char*)errorMessage->GetBufferPointer(), "PS compile error", MB_OK);
		}
		return false;
	}

	Device->CreatePixelShader(pixelShaderCSO->GetBufferPointer(),
		pixelShaderCSO->GetBufferSize(), nullptr, &PixelShader);

	pixelShaderCSO->Release();

	return true;
}

bool UShader::CreateConstBuffer()
{
	HRESULT result;

	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.ByteWidth = sizeof(FConstants) + 0xf & 0xfffffff0;	// ensure constant buffer size is multiple of 16 bytes
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;	// will be updated from CPU every frame
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	result = Device->CreateBuffer(&constantBufferDesc, nullptr, &ConstantBuffer);
	if (FAILED(result))
	{
		MessageBox(nullptr, L"constbuffer create fail,", L"error", MB_OK);
		return false;
	}

	return true;
}
