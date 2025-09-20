#pragma once

#include "Actor/Actor.h"

class UCubeComponent;
class UTextComponent;
class ACubeActor : public AActor
{
	DECLARE_CLASS(ACubeActor, AActor)
public:
	ACubeActor();

private:
	UCubeComponent* CubeComponent = nullptr;
	UTextComponent* TextComponent = nullptr;
};
