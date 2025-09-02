#include "ObjectFactory.h"

UObjectFactory* UObjectFactory::instance = nullptr;
UObjectFactory* UObjectFactory::GetInstance()
{
	if (instance == nullptr)
	{
		instance = new UObjectFactory();
	}
	return instance;
}

UObjectFactory::~UObjectFactory()
{
	ReleaseAll();
}

void UObjectFactory::ReleaseAll()
{
	for (UINT i = 0; i < ObjectListSize; ++i)
	{
		if (ObjectList[i])
		{
			delete ObjectList[i];
			ObjectList[i] = nullptr;
		}
	}
	if (ObjectList)
	{
		delete[] ObjectList;
		ObjectList = nullptr;
	}
	ObjectListSize = 0;
}

void UObjectFactory::Update(float deltaTime)
{
	for (UINT i = 0; i < ObjectListSize; ++i)
	{
		ObjectList[i]->Update(deltaTime);
	}
}

void UObjectFactory::Render()
{
	for (UINT i = 0; i < ObjectListSize; ++i)
	{
		if (ObjectList[i])
		{
			// Render
		}
	}
}

UObject* UObjectFactory::CreatePlayer(FVector3 location)
{
	UPlayer* newObject = new UPlayer();
	newObject->SetLocation(location);
	AddObject(newObject);
	return newObject;
}

void UObjectFactory::AddObject(UObject* newObject)
{
	if (!newObject) return;
	UObject** newPrimitiveList = new UObject * [ObjectListSize + 1];

	for (UINT i = 0; i < ObjectListSize; ++i)
	{
		newPrimitiveList[i] = ObjectList[i];

	}
	newPrimitiveList[ObjectListSize] = newObject;
	if (ObjectList)
	{
		delete[] ObjectList;
	}
	ObjectList = newPrimitiveList;
	ObjectListSize++;
}
