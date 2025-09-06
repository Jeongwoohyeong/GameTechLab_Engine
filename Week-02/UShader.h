#pragma once
#include "Math.h"

class ID3DDevice;
class ID3D11DeviceContext;
class ID3D11VertexShader;
class ID3D11PixelShader;
class ID3D11InputLayout;
class ID3D11Buffer;

//******************************************/
//
//	UpdateConstant 미완성
//
//
//******************************************/


struct FConstants
{
	FVector offset;
	float Scale;
};

class UShader
{
public:
	UShader() {};
	~UShader() {};

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*);
	void PrepareShader();
	void UpdateContant(FVector location, float scale);
	void Release();

private:
	bool CreateVertexShader();
	bool CreatePixelShader();
	bool CreateConstBuffer();	

private:
	ID3D11Device* Device = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;
	ID3D11VertexShader* VertexShader = nullptr;
	ID3D11PixelShader* PixelShader = nullptr;
	ID3D11InputLayout* InputLayout = nullptr;
	ID3D11Buffer* ConstantBuffer = nullptr;	
};