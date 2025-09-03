#pragma once
#include "Object.h"
#include "Vector.h"
#include "Player.h"
#include "CustomVector.h"

class UObjectFactory
{
	typedef unsigned int UINT;

public:
	static UObjectFactory* GetInstance();

private:
	static UObjectFactory* instance;

	UCustomVector<UObject*> ObjectList;

public:
	UObjectFactory() = default;
	~UObjectFactory();

	void ReleaseAll();
	void Release(UObject* Object);
	void Update(float deltaTime);

	void Render();

	// Factory
public:
	UObject* CreatePlayer(int newPlyaerIndex, FVector3 location, FVector3 scale = FVector3(1.0f, 1.0f, 1.0f));
	UObject* CreateBall(FVector3 location, FVector3 scale = FVector3(1.0f, 1.0f, 1.0f));
	UObject* CreateWall(FVector3 location, FVector3 scale = FVector3(1.0f, 1.0f, 1.0f));
	UObject* CreatePunch(FVector3 location, FVector3 scale = FVector3(1.0f, 1.0f, 1.0f));

private:
	void AddObject(UObject* newObject);
};