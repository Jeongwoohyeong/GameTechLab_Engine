#include "pch.h"
#include "SpotlightComponent.h"
#include "DecalComponent.h"

USpotlightComponent::USpotlightComponent()
{
	// 프로퍼티의 기본값을 설정합니다.
	ConeAngle = 90.0f;
	AttenuationRadius = 50.0f;
	LightColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
	Intensity = 1.0f;
	bCanEverTick = true;

}

USpotlightComponent::~USpotlightComponent()
{
	DeleteObject(DecalComponent);
	DecalComponent = nullptr;
}

FVector USpotlightComponent::CalculateScale(float InConeAngle, float InAttenuationRadius)
{
	// Spotlight cone 반지름 = tan(ConeAngle/2) × AttenuationRadius
	float PlaneScale = tan(DegreeToRadian(InConeAngle / 2.0f)) * InAttenuationRadius;
	return FVector(
		InAttenuationRadius, PlaneScale, PlaneScale
	);
}

void USpotlightComponent::Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj)
{
	if(DecalComponent)
		DecalComponent->Render(Renderer, View, Proj);
}

void USpotlightComponent::InitializeComponent()
{
	Super_t::InitializeComponent();

	// 자식으로 사용할 DecalComponent를 생성하고 붙입니다.
	if (Owner && !DecalComponent)
	{
		DecalComponent = Cast<UDecalComponent>(
			Owner->CreateAndAttachComponent(this, UDecalComponent::StaticClass()));

		if (DecalComponent)
		{
			// Decal 컴포넌트의 초기 설정
			// TODO: 적절한 스포트라이트 텍스처로 변경 (현재는 기본 Decal 텍스처 사용)
			DecalComponent->SetUsePerspectiveProjection(true);
			DecalComponent->SetTexture("Editor/Decal/spotlight_gaussian_edge0.dds");
		}
	}
}

void USpotlightComponent::TickComponent(float DeltaTime)
{
	Super_t::TickComponent(DeltaTime);

	if (DecalComponent)
	{
		DecalComponent->SetRelativeScale(CalculateScale(ConeAngle, AttenuationRadius));
		//DecalComponent->SetRelativeRotation(FQuat::MakeFromEuler(FVector(0, 90, 0)));
		DecalComponent->SetRelativeLocation(FVector(AttenuationRadius / 2.0f, 0, 0));
		DecalComponent->SetProjectionFOV(ConeAngle);
	}
}

float USpotlightComponent::GetConeAngle() const
{
	return ConeAngle;
}

void USpotlightComponent::SetConeAngle(float InConeAngle)
{
	ConeAngle = InConeAngle;
}

float USpotlightComponent::GetAttenuationRadius() const
{
	return AttenuationRadius;
}

void USpotlightComponent::SetAttenuationRadius(float InAttenuationRadius)
{
	AttenuationRadius = InAttenuationRadius;
}

UObject* USpotlightComponent::Duplicate()
{
	USpotlightComponent* DuplicatedComponent = Cast<USpotlightComponent>(NewObject(GetClass()));
	if (DuplicatedComponent)
	{
		CopyCommonProperties(DuplicatedComponent);

		// Spotlight 프로퍼티 복사
		DuplicatedComponent->ConeAngle = this->ConeAngle;
		DuplicatedComponent->AttenuationRadius = this->AttenuationRadius;
		DuplicatedComponent->LightColor = this->LightColor;
		DuplicatedComponent->Intensity = this->Intensity;

		// DecalComponent는 InitializeComponent에서 생성되므로 여기서는 복사하지 않습니다.
		DuplicatedComponent->DuplicateSubObjects();
	}

	return DuplicatedComponent;
}