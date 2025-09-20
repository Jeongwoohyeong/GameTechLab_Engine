#include "pch.h"
#include "Actor/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextComponent.h"

IMPLEMENT_CLASS(AStaticMeshActor, AActor)

AStaticMeshActor::AStaticMeshActor()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	StaticMeshComponent->SetOwner(this);
	SetRootComponent(StaticMeshComponent);

	TextComponent = CreateDefaultSubobject<UTextComponent>("TextComponent");
	TextComponent->SetOwner(this);
	TextComponent->SetParentAttachment(StaticMeshComponent);


}
