#include "ObjectFactory.h"
#include "UMeshRenderer.h"
#include "App.h"
#include "Ball.h"

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

		for (UINT j = i + 1; j < ObjectListSize; ++j)
		{
			UObject* objA = ObjectList[i];
			UObject* objB = ObjectList[j];

			UPhysicsComponent* physicsA = objA->GetPhysicsComponent();
			UPhysicsComponent* physicsB = objB->GetPhysicsComponent();

			if (physicsA && physicsB)
			{
				if (physicsA->CheckCollision(physicsB))
				{
					physicsA->OnCollision(physicsB);
					physicsB->OnCollision(physicsA);
				}
			}
		}
	}
}

void UObjectFactory::Render()
{
	//������ ���� �׸��� ���� �ʿ�
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

UObject* UObjectFactory::CreateBall(FVector3 location)
{
	UBall* newObject = new UBall(new UMeshRenderer(new UMesh(FMeshData::CircleMeshData)));
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
