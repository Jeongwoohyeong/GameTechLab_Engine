#include "pch.h"
#include "PostProcessFXAA.h"
#include "Shader.h"

FPostProcessFXAA::FPostProcessFXAA()
{
    FXAAShader = UResourceManager::GetInstance().Load<UShader>("FXAAShader.hlsl");
}

FPostProcessFXAA::FPostProcessFXAA(URHIDevice* InRHIDevice)
    : RHIDevice(InRHIDevice)
{
    FXAAShader = UResourceManager::GetInstance().Load<UShader>("FXAAShader.hlsl");    
}

FPostProcessFXAA::~FPostProcessFXAA()
{
}

void FPostProcessFXAA::Render(ID3D11ShaderResourceView* InSceneTextureSRV)
{
    if (!FXAAShader)
    {
        return;
    }

    PrepareShader();
    RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &InSceneTextureSRV);
    RHIDevice->PSSetDefaultSampler(0);
    RHIDevice->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    RHIDevice->GetDeviceContext()->Draw(3,0);

    ID3D11ShaderResourceView* NullSRV = nullptr;
    RHIDevice->GetDeviceContext()->PSSetShaderResources(0, 1, &NullSRV);
}

void FPostProcessFXAA::PrepareShader() const
{
    ID3D11DeviceContext* DeviceContext = RHIDevice->GetDeviceContext();
    DeviceContext->VSSetShader(FXAAShader->GetVertexShader(), nullptr, 0);
    DeviceContext->PSSetShader(FXAAShader->GetPixelShader(), nullptr, 0);
    DeviceContext->IASetInputLayout(nullptr);
    
}
