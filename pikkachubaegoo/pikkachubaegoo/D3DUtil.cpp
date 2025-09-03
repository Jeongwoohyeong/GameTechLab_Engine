#include "D3DUtil.h"
#include "App.h"

void D3DUtil::CreateVSAndInputLayout(LPCWSTR& fileName, ID3D11VertexShader** vs, ID3D11InputLayout** inputLayout)
{
	ID3DBlob* vsCSO;
	HRESULT hResult = D3DCompileFromFile(fileName, nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &vsCSO, nullptr);
	if (FAILED(hResult))
	{
		wcout << "VS Compile Failed. FileName : " << fileName << endl;
		return;
	}
	hResult = UApp::Ins->GetDevice()->CreateVertexShader(vsCSO->GetBufferPointer(), vsCSO->GetBufferSize(), nullptr, vs);

	if (FAILED(hResult))
	{
		wcout << "Create VS Failed. FileName : " << fileName << endl;
		return;
	}

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	
	hResult = UApp::Ins->GetDevice()->CreateInputLayout(layout, ARRAYSIZE(layout), vsCSO->GetBufferPointer(), vsCSO->GetBufferSize(), inputLayout);

	if (FAILED(hResult))
	{
		wcout << "Create InputLayout Failed. FileName : " << fileName << endl;
		return;
	}
}

void D3DUtil::CreatePS(LPCWSTR& fileName, ID3D11PixelShader** ps)
{
	ID3DBlob* vsCSO;
	HRESULT hResult = D3DCompileFromFile(fileName, nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &vsCSO, nullptr);
	if (FAILED(hResult))
	{
		wcout << "PS Compile Failed. FileName : " << fileName << endl;
		return;
	}
	hResult = UApp::Ins->GetDevice()->CreatePixelShader(vsCSO->GetBufferPointer(), vsCSO->GetBufferSize(), nullptr, ps);

	if (FAILED(hResult))
	{
		wcout << "Create PS Failed. FileName : " << fileName << endl;
		return;
	}
}

void D3DUtil::CreateConstantBuffer(ID3D11Buffer** cBuffer, UINT size)
{
	D3D11_BUFFER_DESC constantbufferdesc = {};
	constantbufferdesc.ByteWidth = size + 0xf & 0xfffffff0; //16byte 단위로 끊기위한 작업 ((size + 15) / 16)
	constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	UApp::Ins->GetDevice()->CreateBuffer(&constantbufferdesc, nullptr, cBuffer);
}
