#include "pch.h"
#include "FireBallComponent.h"
#include "Renderer.h"
#include "World.h" 
UFireBallComponent::UFireBallComponent() : Intensity(10.f), Radius(50.f), RadiusFallOff(2.f), Color(FLinearColor(1.f, .5f, .1f, 1.f))
{
}
void UFireBallComponent::Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj)
{
	//FVector4 WorldLocation(GetWorldLocation().X, GetWorldLocation().Y, GetWorldLocation().Z, 1.0f);

	/*const FMatrix YUpToZUp =
	{
		 0,  1,  0, 0 ,
		 0,  0,  1, 0 ,
		 1, 0,  0, 0 ,
		 0,  0,  0, 1
	};
	
	WorldLocation = YUpToZUp * WorldLocation;*/

	FVector4 WorldLocation = FVector4(0.0f, 0.0f, 0.0f, 1.0f) * GetRealWorldMatrix();

	float InvRadius = Radius > 0.f ? 1.f / Radius : 0.f;
	FVector4 Parameters(Intensity, Radius, InvRadius, RadiusFallOff);
	FireBallBufferType FireBallData = { WorldLocation, Color, Parameters };
	Renderer->AddFireBallToScene(FireBallData);
}

void UFireBallComponent::AddDebugLine(URenderer* Renderer)
{
	// 에디터에서만 보이도록 디버그 렌더링 처리
	if (GetWorld() && !GetWorld()->IsPIEWorld())
	{
		const FVector Center = GetWorldLocation();
		const float Radius = GetRadius();
		const int32 Segments = 32; // 구를 표현할 선분의 개수

		// 3개의 축(XY, XZ, YZ)에 대한 원을 그려 구를 표현합니다.
		FVector OldPoint_XY = Center + FVector(Radius, 0.f, 0.f);
		FVector OldPoint_XZ = Center + FVector(Radius, 0.f, 0.f);
		FVector OldPoint_YZ = Center + FVector(0.f, Radius, 0.f);

		for (int32 i = 1; i <= Segments; ++i)
		{
			const float Angle = static_cast<float>(i) / Segments * 2.0f * PI;
			const float Sin = sinf(Angle);
			const float Cos = cosf(Angle);

			// XY 평면의 원
			FVector NewPoint_XY = Center + FVector(Radius * Cos, Radius * Sin, 0.f);
			Renderer->AddLine(OldPoint_XY, NewPoint_XY);
			OldPoint_XY = NewPoint_XY;

			// XZ 평면의 원
			FVector NewPoint_XZ = Center + FVector(Radius * Cos, 0.f, Radius * Sin);
			Renderer->AddLine(OldPoint_XZ, NewPoint_XZ);
			OldPoint_XZ = NewPoint_XZ;

			// YZ 평면의 원
			FVector NewPoint_YZ = Center + FVector(0.f, Radius * Cos, Radius * Sin);
			Renderer->AddLine(OldPoint_YZ, NewPoint_YZ);
			OldPoint_YZ = NewPoint_YZ;
		}
	}
}

void UFireBallComponent::Serialize(FObjectData* Data)
{
	FFireBallComponentData* ComponentData = dynamic_cast<FFireBallComponentData*>(Data);
	assert(ComponentData && "UFireBallComponent::Serialize got wrong data type.");
	
	// 부모 클래스의 데이터를 먼저 직렬화
	USceneComponent::Serialize(Data);
	
	// FireBallComponent 고유의 데이터를 직렬화합니다.
	ComponentData->Color = Color;
	ComponentData->Intensity = Intensity;
	ComponentData->Radius = Radius;
	ComponentData->FallOff = RadiusFallOff;
}

void UFireBallComponent::DeSerialize(FObjectData* Data)
{
	FFireBallComponentData* ComponentData = dynamic_cast<FFireBallComponentData*>(Data);
	assert(ComponentData && "UFireBallComponent::DeSerialize got wrong data type.");
	
	// 부모 클래스의 데이터를 먼저 역직렬화
	USceneComponent::DeSerialize(Data);
	
	// FireBallComponent 고유의 데이터를 역직렬화
	Color = ComponentData->Color;
	Intensity = ComponentData->Intensity;
	Radius = ComponentData->Radius;
	RadiusFallOff = ComponentData->FallOff;
}

UObject* UFireBallComponent::Duplicate()
{
	UFireBallComponent* DuplicatedComponent = Cast<UFireBallComponent>(NewObject(GetClass()));
	CopyCommonProperties(DuplicatedComponent);

	DuplicatedComponent->Color = this->Color;
	DuplicatedComponent->Intensity = this->Intensity;
	DuplicatedComponent->Radius = this->Radius;
	DuplicatedComponent->RadiusFallOff = this->RadiusFallOff;

	DuplicatedComponent->DuplicateSubObjects();
	return DuplicatedComponent;
}

void UFireBallComponent::DuplicateSubObjects()
{
	Super_t::DuplicateSubObjects();
}
