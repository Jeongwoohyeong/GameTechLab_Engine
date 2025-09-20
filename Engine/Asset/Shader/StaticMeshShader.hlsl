cbuffer PerFrame : register(b1)
{
	row_major float4x4 ViewMatrix;
	row_major float4x4 ProjectionMatrix;
	uint ViewModeIndex;
	float3 Padding;
};

//아래의 constantbuffer는 여러 종류의 매시를 그릴 때 스트럭처드 버퍼를 한번만 업데이트해서 그리기 위해 존재함.
//지금은 매시 종류가 10가지도 안되므로 오버엔지니어링이라고 생각해서 주석처리함.
//cbuffer InstanceParams : register(b3)
//{
//	uint UseInstancing;
//	uint BaseInstanceOffset; //baseOffset
//	uint InstanceCount; //인스턴스 개수
//	uint Padding0;
//};

struct InstanceData
{
	row_major float4x4 World;
	float4 Color;
};

StructuredBuffer<InstanceData> InstanceMatrices : register(t0);

struct VS_INPUT
{
	float4 Position : POSITION;
	float3 Normal : NORMAL;
	float4 Color : COLOR;
	float2 BaseUV : TEXTURE;
};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float2 UV : TEXTURE;
};

Texture2D Texture : register(t1);
SamplerState Sampler : register(s0);

//InstanceID는 GPU가 알아서 세팅해줌.(CPU에서 넘긴 인스턴스 버퍼의 데이터 순서대로 0부터 InstanceCount까지 알아서 전달)
//SV_InstanceID 태그로 식별
PS_INPUT MainVS(VS_INPUT Input, uint InstanceId : SV_InstanceID)
{
	PS_INPUT Output;

	float4 Position = Input.Position;
	float4 ShadeColor = Input.Color;

	
	//아래 주석은 InstanceParam 확인
    //if (UseInstancing != 0 && InstanceId < InstanceCount)
    //{
    //    InstanceData Instance = InstanceMatrices[BaseInstanceOffset + InstanceId];
    //    Position = mul(Position, Instance.World);
    //    ShadeColor = lerp(ShadeColor, Instance.Color, Instance.Color.a);
    //}

	//현재 에디터 라인 배치를 할때 같은 셰이더를 쓰기 때문에 Instancing을 사용하는지 확인이 필요함.
	//나중에 라인 배치는 다른 셰이더를 사용하고 가독성을 위해 아래 코드를 지우는 게 나을 것 같음.

	
	InstanceData Instance = InstanceMatrices[InstanceId];
	Position = mul(Position, Instance.World);
		
	Position = mul(Position, ViewMatrix);
	Position = mul(Position, ProjectionMatrix);

	Output.Position = Position;
	Output.Color = ShadeColor;
	Output.UV = Input.BaseUV;
	return Output;
}

float4 MainPS(PS_INPUT Input) : SV_TARGET
{
	//float4 TextureColor = Texture.Sample(Sampler, Input.UV);
	return Input.Color;
}
