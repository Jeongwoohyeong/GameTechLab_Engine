#pragma once

struct FFXAABufferType;
class URHIDevice;
class UShaader;
class URenderer;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;

class FPostProcessFXAA
{    
public:
    FPostProcessFXAA();
    FPostProcessFXAA(URHIDevice* InRHIDevice);    
    ~FPostProcessFXAA();

    void Render(ID3D11ShaderResourceView* InSceneTextureSRV);

private:
    void PrepareShader() const;

private:
    URHIDevice* RHIDevice = nullptr;
    UShader* FXAAShader = nullptr;
    
};
