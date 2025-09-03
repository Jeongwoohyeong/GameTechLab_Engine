#include "ObjectFactory.h"
#include "UMeshRenderer.h"
#include "App.h"
#include "Ball.h"
#include "Wall.h"

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
	int objListCount = ObjectList.Size();
	for (UINT i = 0; i < objListCount; ++i)
	{
		delete ObjectList[i];
	}
	ObjectList.Clear();
}

void UObjectFactory::Update(float deltaTime)
{
	UINT objListCount = ObjectList.Size();
	for (UINT i = 0; i < objListCount; ++i)
	{
		ObjectList[i]->Update(deltaTime);

		for (UINT j = i + 1; j < objListCount; ++j)
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
	UINT objListCount = ObjectList.Size();

	for (UINT i = 0; i < objListCount; ++i)
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
	UBall* newObject = new UBall(new UMeshRenderer(UApp::Ins->GetCircleMesh()));
	newObject->SetLocation(location);
	AddObject(newObject);
	return newObject;
}

UObject* UObjectFactory::CreateWall(FVector3 location)
{
	UWall* newObject = new UWall(new UMeshRenderer(new UMesh(FMeshData::CircleMeshData)));
	newObject->SetLocation(location);
	AddObject(newObject);
	return newObject;
}

void UObjectFactory::AddObject(UObject* newObject)
{
	if (!newObject) return;
	ObjectList.PushBack(newObject);
}
