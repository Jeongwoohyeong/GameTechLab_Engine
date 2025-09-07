#pragma once
#include "Math.h"

class ID3DDevice;
struct ID3D11DeviceContext;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11Buffer;

//******************************************/
//
//	UpdateConstant 미완성
//
//
//******************************************/


struct FConstants
{
	FMatrix MVPMatrix;
};

class UShader
{
public:
	UShader() {};
	~UShader() {};

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*);
	void PrepareShader();
	void UpdateConstant(const FMatrix&);
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