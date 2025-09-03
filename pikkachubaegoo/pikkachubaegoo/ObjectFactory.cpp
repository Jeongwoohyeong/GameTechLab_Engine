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
	//렌더되는 오브젝트만 추출
	UINT objListCount = ObjectList.Size();
	UCustomVector<UObject*> renderObjects;
	for (UINT i = 0; i < objListCount; ++i)
	{
		if (ObjectList[i]->GetRenderer() != nullptr)
		{
			renderObjects.PushBack(ObjectList[i]);
		}
	}
	UINT renderObjectCount = renderObjects.Size();
	
	//렌더되는 오브젝트 order순서에 따라 정렬 0 ~ 9999 오름차순
	for (UINT i = 0; i < renderObjectCount - 1; ++i)
	{
		for (UINT j = 0; j < renderObjectCount - i - 1; ++j)
		{
			UObject* prev = renderObjects[j];
			UObject* next = renderObjects[j + 1];
			UINT prevDrawOrder = prev->GetRenderer()->GetDrawOrder();
			UINT nextDrawOrder = next->GetRenderer()->GetDrawOrder();

			if (prevDrawOrder > nextDrawOrder)
			{
				renderObjects[j] = next;
				renderObjects[j + 1] = prev;
			}
		}
	}

	//순서에 맞게 렌더링
	for (UINT i = 0; i < renderObjectCount; ++i)
	{
		renderObjects[i]->Draw();
	}
}

UObject* UObjectFactory::CreatePlayer(FVector3 location, FVector3 scale)
{
	UPlayer* newObject = new UPlayer(new UMeshRenderer(UApp::Ins->GetQuadMesh()));
	newObject->GetTransform()->SetLocation(location);
	newObject->GetTransform()->SetScale(scale);
	AddObject(newObject);
	return newObject;
}

UObject* UObjectFactory::CreateBall(FVector3 location, FVector3 scale)
{
	UBall* newObject = new UBall(new UMeshRenderer(UApp::Ins->GetCircleMesh()));
	newObject->GetTransform()->SetLocation(location);
	newObject->GetTransform()->SetScale(scale);
	AddObject(newObject);
	return newObject;
}

UObject* UObjectFactory::CreateWall(FVector3 location, FVector3 scale)
{
	UWall* newObject = new UWall(new UMeshRenderer(UApp::Ins->GetQuadMesh()));
	newObject->GetTransform()->SetLocation(location);
	newObject->GetTransform()->SetScale(scale);
	AddObject(newObject);
	return newObject;
}

void UObjectFactory::AddObject(UObject* newObject)
{
	if (!newObject) return;
	ObjectList.PushBack(newObject);
}
