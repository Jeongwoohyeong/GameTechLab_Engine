#include "pch.h"
#include "FireBallComponent.h"
UFireBallComponent::UFireBallComponent() : Intensity(10.f), Radius(50.f), RadiusFallOff(2.f), Color(FLinearColor(1.f, .5f, .1f, 1.f))
{
	//Super_t::SetMaterial("FireBallShader.hlsl");
}

void UFireBallComponent::Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj)
{
	FVector4 WorldLocation(GetWorldLocation().X, GetWorldLocation().Y, GetWorldLocation().Z, 1.0f);
	float InvRadius = Radius > 0.f ? 1.f / Radius : 0.f;
	FVector4 Parameters(Intensity, Radius, InvRadius, RadiusFallOff);
	FireBallBufferType FireBallData = { WorldLocation, Color, Parameters };

	Renderer->AddFireBallToScene(FireBallData);
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
