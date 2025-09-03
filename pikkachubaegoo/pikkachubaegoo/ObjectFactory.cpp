#include "ObjectFactory.h"
#include "UMeshRenderer.h"
#include "App.h"

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
	//순서에 따른 그리기 정렬 필요
	for (UINT i = 0; i < ObjectListSize; ++i)
	{
		if (ObjectList[i])
		{
			ObjectList[i]->Draw();
		}
	}
}

UObject* UObjectFactory::CreatePlayer(FVector3 location)
{
	UPlayer* newObject = new UPlayer(new UMeshRenderer(UApp::Ins->GetQuadMesh()));
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
