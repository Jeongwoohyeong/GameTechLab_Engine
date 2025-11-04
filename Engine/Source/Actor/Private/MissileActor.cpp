#include "pch.h"
#include "Actor/Public/MissileActor.h"
#include "Component/Mesh/Public/StaticMeshComponent.h"
#include "Component/Collision/Public/SphereComponent.h"
#include "Component/Public/SceneComponent.h"
#include "Component/Public/ULuaScriptComponent.h"

IMPLEMENT_CLASS(AMissileActor, AActor)

AMissileActor::AMissileActor()
{
	bCanEverTick = true;
	// 메쉬 컴포넌트 생성
	StaicMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>();
	if (StaicMeshComponent)
	{
		SetRootComponent(StaicMeshComponent);
		StaicMeshComponent->SetStaticMesh("Data/Missile.obj");
		StaicMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
		StaicMeshComponent->SetRelativeRotation(FQuaternion::FromEuler(FVector(0.0f, 0.0f, 0.0f)));
		StaicMeshComponent->SetRelativeScale3D(FVector(50.f, 200.f, 200.f));
		StaicMeshComponent->SetCanEverTick(false);
		StaicMeshComponent->bGenerateOverlapEvents = false;
	}
	// 구체 충돌 컴포넌트 생성
	MissileCollision = CreateDefaultSubobject<USphereComponent>();
	if (MissileCollision)
	{
		MissileCollision->AttachToComponent(StaicMeshComponent);
		MissileCollision->SetSphereRadius(0.03f);
		MissileCollision->SetRelativeLocation(FVector(0.45f, 0.0f, 0.0f));
		MissileCollision->bGenerateHitEvents = true;
		MissileCollision->bBlockComponent = true;  // ✅ Hit 이벤트를 위해 Blocking 활성화
		MissileCollision->SetCanEverTick(true);
		MissileCollision->OnComponentHit.AddDynamic(this, &AMissileActor::OnMissileHit);
	}
}

AMissileActor::~AMissileActor()
{
}

void AMissileActor::BeginPlay()
{
	Super::BeginPlay();

	if (MissileCollision)
	{
		RegisterComponent(MissileCollision);
	}
}

void AMissileActor::OnMissileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 자기 자신과의 충돌 무시
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	// 플레이어와의 충돌 무시
	FString OtherName = OtherActor->GetName().ToString();
	if (OtherName.find("APlayerCharacter") != std::string::npos)
	{
		return;
	}

	// 다른 미사일과의 충돌 무시
	if (OtherName.find("AMissileActor") != std::string::npos)
	{
		return;
	}

	// Lua 스크립트에 충돌 이벤트 전달 (OnHit으로 변경)
	if (ULuaScriptComponent* LuaComp = GetLuaScriptComponent())
	{
		LuaComp->ActivateFunction("OnHit", HitComponent, OtherActor,
			OtherComp, NormalImpulse, Hit);
	}
}
