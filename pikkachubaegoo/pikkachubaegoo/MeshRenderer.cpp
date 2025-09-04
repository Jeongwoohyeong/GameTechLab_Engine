#include "MeshRenderer.h"
#include "App.h"
const UINT UMeshRenderer::PikkachuOrder = 100;
const UINT UMeshRenderer::BallOrder = 110;
const UINT UMeshRenderer::BallTrailOrder = 109;
const UINT UMeshRenderer::NetOrder = 99;
const UINT UMeshRenderer::PunchOrder = 98;
const UINT UMeshRenderer::BGSkyOrder = 0;
const UINT UMeshRenderer::BGGroundOrder = 2;
const UINT UMeshRenderer::BGMountainOrder = 1;

UMeshRenderer::UMeshRenderer(UMesh* InMesh, UINT InDrawOrder, ID3D11ShaderResourceView* textureSRV)
{
	Mesh = InMesh;
	SetDrawOrder(InDrawOrder);
	D3DUtil::CreateConstantBuffer(&AtlasInfoCBuffer, sizeof(FVector4));
	SetTextureSRV(textureSRV);
}


UMeshRenderer::~UMeshRenderer()
{
	if (Mesh)
	{
		//Mesh->Release();	게임 종료시 UApp에서 Release 호출해줘서 주석 처리
		Mesh = nullptr;
	}
	if (AtlasInfoCBuffer)
	{
		AtlasInfoCBuffer->Release();
	}
}
void UMeshRenderer::ChangeAtlasInfo(const FVector4& atlasInfo)
{
	AtlasInfo = atlasInfo;
	D3DUtil::UpdateConstantBuffer(UApp::Ins->GetContext(), AtlasInfoCBuffer, AtlasInfo);
}
void UMeshRenderer::SetShader(ID3D11InputLayout* inputLayout, ID3D11VertexShader* vs, ID3D11PixelShader* ps)
{
	InputLayout = inputLayout;
	VS = vs;
	PS = ps;
}

void UMeshRenderer::DrawMesh(const FMatrix4x4& Transform)
{
	// 실제 메시를 그리는 로직 구현
	static ID3D11VertexShader* lastVS = nullptr;
	static ID3D11PixelShader* lastPS = nullptr;
	static ID3D11InputLayout* lastInputLayout = nullptr;
	static ID3D11ShaderResourceView* lastSRV = nullptr;

	if (Mesh)
	{
		if (lastVS == nullptr)
		{
			UApp::Ins->GetContext()->IASetInputLayout(InputLayout);
			UApp::Ins->GetContext()->VSSetShader(VS, 0, 0);
			UApp::Ins->GetContext()->PSSetShader(PS, 0, 0);
			UApp::Ins->GetContext()->PSSetShaderResources(0, 1, &TextureSRV);
		}
		else
		{
			if (lastVS != VS)
			{
				UApp::Ins->GetContext()->VSSetShader(VS, 0, 0);
			}
			if (lastPS != PS)
			{
				UApp::Ins->GetContext()->VSSetShader(VS, 0, 0);
			}
			if (lastInputLayout != InputLayout)
			{
				UApp::Ins->GetContext()->IASetInputLayout(InputLayout);
			}
			if (lastSRV != TextureSRV)
			{
				UApp::Ins->GetContext()->PSSetShaderResources(0, 1, &TextureSRV);
			}
		}
		D3DUtil::UpdateConstantBuffer(UApp::Ins->GetContext(), UApp::Ins->GetTransformCBuffer(), Transform);
		UApp::Ins->GetContext()->VSSetConstantBuffers(1, 1, &AtlasInfoCBuffer);


		Mesh->Draw();
	}
}
