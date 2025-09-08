// ShaderW0.hlsl

cbuffer constants : register(b0)
{
    row_major float4x4 MVP;
    float3 color;
    int useUColor;
}

struct VS_INPUT
{
    float4 position : POSITION; // Input position from vertex buffer
    float4 color : COLOR; // Input color from vertex buffer
};

struct PS_INPUT
{
    float4 position : SV_POSITION; // Transformed position to pass to the pixel shader
    float4 color : COLOR; // Color to pass to the pixel shader
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;    
    
    // mvp transformation
    output.position = mul(input.position, MVP);
    
    // Pass the color to the pixel shader
    if (useUColor == 1)
        output.color = float4(color, 1.0);
    else
        output.color = input.color;
    
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    // Output the color directly
    return input.color;
}
