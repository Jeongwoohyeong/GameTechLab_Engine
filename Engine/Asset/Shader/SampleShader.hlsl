cbuffer constants : register(b0)
{
    row_major float4x4 world;
}

cbuffer PerFrame : register(b1)
{
    row_major float4x4 ViewMatrix;
    row_major float4x4 ProjectionMatrix;
    uint ViewModeIndex;
    float3 Padding;
};

cbuffer PerDrawColor : register(b2)
{
    float4 totalColor;
};

cbuffer InstanceParams : register(b3)
{
    uint UseInstancing;
    uint BaseInstanceOffset;	//baseOffset
    uint InstanceCount;	//인스턴스 개수
    uint Padding0;
};

struct InstanceData
{
    row_major float4x4 World;
    float4 Color;
};

StructuredBuffer<InstanceData> InstanceMatrices : register(t0);

struct VS_INPUT
{
    float4 Position : POSITION;
    float4 Color : COLOR;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

//InstanceID는 GPU가 알아서 세팅해줌.(CPU에서 넘긴 인스턴스 버퍼의 데이터 순서대로 0부터 InstanceCount까지 알아서 전달)
//SV_InstanceID 태그로 식별
PS_INPUT MainVS(VS_INPUT Input, uint InstanceId : SV_InstanceID)
{
    PS_INPUT Output;

    float4 Position = Input.Position;
    float4 ShadeColor = Input.Color;

    if (UseInstancing != 0 && InstanceId < InstanceCount)
    {
        InstanceData Instance = InstanceMatrices[BaseInstanceOffset + InstanceId];
        Position = mul(Position, Instance.World);
        ShadeColor = lerp(ShadeColor, Instance.Color, Instance.Color.a);
    }
	//인스턴싱 하는 경우 어차피 World는 Identity임.
    Position = mul(Position, world);
    Position = mul(Position, ViewMatrix);
    Position = mul(Position, ProjectionMatrix);

    Output.Position = Position;
    Output.Color = ShadeColor;
    return Output;
}

float4 MainPS(PS_INPUT Input) : SV_TARGET
{
    float4 FinalColor = lerp(Input.Color, totalColor, totalColor.a);
    return FinalColor;
}
