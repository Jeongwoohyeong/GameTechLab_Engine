#include "pch.h"
#include "Actor/CubeActor.h"
#include "Mesh/TextComponent.h"
#include "Mesh/PrimitiveComponent.h"
IMPLEMENT_CLASS(ACubeActor, AActor)

ACubeActor::ACubeActor()
{
	CubeComponent = CreateDefaultSubobject<UCubeComponent>("CubeComponent");
	CubeComponent->SetOwner(this);
	TextComponent = CreateDefaultSubobject<UTextComponent>("TextComponent");
	TextComponent->SetOwner(this);
	TextComponent->SetParentAttachment(CubeComponent);
	SetRootComponent(CubeComponent);
}
