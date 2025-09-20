#pragma once

#include "Actor/Actor.h"

class UStaticMeshComponent;
class UTextComponent;

class AStaticMeshActor : public AActor
{
	DECLARE_CLASS(AStaticMeshActor, AActor);

public:
	AStaticMeshActor();


private:
	UStaticMeshComponent* StaticMeshComponent = nullptr;
	UTextComponent* TextComponent = nullptr;
};

