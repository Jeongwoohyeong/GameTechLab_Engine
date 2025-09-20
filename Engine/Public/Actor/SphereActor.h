#pragma once

#include "Actor/Actor.h"

class USphereComponent;
class UTextComponent;
class ASphereActor : public AActor
{
	DECLARE_CLASS(ASphereActor, AActor)
public:
	ASphereActor();
private:
	USphereComponent* SphereComponent = nullptr;
	UTextComponent* TextComponent = nullptr;
};

