#pragma once
#include "Object.h"
#include "Vector.h"
#include "UPlayer.h"

class UObjectFactory
{
	typedef unsigned int UINT;

public:
	static UObjectFactory* GetInstance();

private:
	static UObjectFactory* instance;

	UObject** ObjectList = nullptr;
	UINT ObjectListSize = 0;

public:
	UObjectFactory() = default;
	~UObjectFactory();

	void ReleaseAll();
	void Update(float deltaTime);

	void Render();

// Factory
public:
	UObject* CreatePlayer(FVector3 location);
	UObject* CreateBall(FVector3 location);
	UObject* CreateWall(FVector3 location);

private:
	void AddObject(UObject* newObject);
};