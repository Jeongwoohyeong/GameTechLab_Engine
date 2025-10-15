#include "pch.h"
#include "FireBallComponent.h"
#include "Renderer.h"
#include "World.h" 
UFireBallComponent::UFireBallComponent() : Intensity(10.f), Radius(10.f), RadiusFallOff(2.f), Color(FLinearColor(1.f, .5f, .1f, 1.f))
{
	bCanEverTick = true;
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
		const int32 Segments = 64; // 구를 표현할 선분의 개수

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

void UFireBallComponent::BeginPlay()
{
	Super_t::BeginPlay();
	OriginalIntensity = Intensity;
	OriginalRadius = Radius;
	OriginalColor = Color;
	StartExplosion();
}

void UFireBallComponent::TickComponent(float DeltaTime)
{
	//float TotalTime = GetWorld()->GetTimeSeconds();

	//// 1. 빛의 세기(Intensity)를 Sin 함수를 이용해 80% ~ 120% 사이에서 변화시킵니다.
	////    Sin 함수의 결과는 -1.0 ~ 1.0 이므로, 0.0 ~ 1.0 범위로 바꾸어 사용합니다.
	//float Pulse = (sin(TotalTime * 2.0f) + 1.0f) * 0.5f; // 0.0 ~ 1.0 사이로 Pulsing
	//float MinIntensity = OriginalIntensity * 0.8f; // OriginalIntensity는 멤버 변수로 저장해둔 초기값
	//float MaxIntensity = OriginalIntensity * 1.2f;
	//SetIntensity(MinIntensity + (MaxIntensity - MinIntensity) * Pulse);

	//// 2. 빛의 색상(Color)을 주황색과 붉은색 사이에서 미세하게 변화시킵니다.
	//FLinearColor OrangeColor(1.0f, 0.5f, 0.1f);
	//FLinearColor RedColor(1.0f, 0.2f, 0.0f);
	//SetColor(FLinearColor::Lerp(OrangeColor, RedColor, Pulse));

	if (bIsExploding)
	{
		// 시간을 계속 누적
		EffectTimer += DeltaTime;

		const float TotalCycleDuration = ExplosionDuration + CooldownDuration;

		//// 1. 폭발 단계 (예: 0초 ~ 1초)
		//if (EffectTimer < ExplosionDuration)
		//{
		//	float Alpha = EffectTimer / ExplosionDuration;

		//	float EaseOutAlpha = 1.0f - pow(1.0f - Alpha, 3.0f);
		//	float FadeAlpha = 1.0f - Alpha;

		//	//SetRadius(OriginalRadius + (OriginalRadius * 4.0f * EaseOutAlpha)); // 기본 크기에서 5배까지 커짐
		//	SetIntensity(OriginalIntensity * 2.0f * FadeAlpha);
		//}
		//// 2. 대기(두근거림) 단계 (예: 1초 ~ 3초)
		//else if (EffectTimer < TotalCycleDuration)
		{
			// 살아 숨 쉬는 듯한 불꽃 효과 (Pulsing)
			float TotalTime = GetWorld()->GetTimeSeconds();

			float Pulse = (sin(TotalTime * 5.0f) + 1.0f) * 0.5f; // 좀 더 빠르게 두근거리도록 속도 증가
			float MinIntensity = OriginalIntensity * 0.5f; // 더 약하게 두근거리도록 범위 조절
			float MaxIntensity = OriginalIntensity * 1.0f;

			SetRadius(OriginalRadius); // 기본 크기 유지
			SetIntensity(MinIntensity + (MaxIntensity - MinIntensity) * Pulse);
			SetColor(FLinearColor::Lerp(OriginalColor, FLinearColor::Red, Pulse));
		}
		//// 3. 사이클 초기화
		//else
		//{
		//	// 타이머를 리셋하여 다음 폭발 사이클을 시작
		//	EffectTimer = 0.0f;
		//}

	}
}

void UFireBallComponent::StartExplosion()
{
	bIsExploding = true;
	EffectTimer = 0.0f;
	ExplosionDuration = 2.0f;
	CooldownDuration = 2.0f;
}
