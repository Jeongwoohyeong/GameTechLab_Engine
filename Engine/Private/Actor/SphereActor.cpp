#include "pch.h"
#include "Actor/SphereActor.h"
#include "Mesh/TextComponent.h"
#include "Mesh/PrimitiveComponent.h"

IMPLEMENT_CLASS(ASphereActor, AActor)
ASphereActor::ASphereActor()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
	SphereComponent->SetOwner(this);
	TextComponent = CreateDefaultSubobject<UTextComponent>("TextComponent");
	TextComponent->SetOwner(this);
	TextComponent->SetParentAttachment(SphereComponent);
	SetRootComponent(SphereComponent);
}
