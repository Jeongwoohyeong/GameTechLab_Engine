#include "pch.h"
#include "StaticMeshComponent.h"
#include "StaticMesh.h"
#include "Shader.h"
#include "Texture.h"
#include "ResourceManager.h"
#include "ObjManager.h"
#include "SceneLoader.h"

UStaticMeshComponent::UStaticMeshComponent()
{
    SetMaterial("StaticMeshShader.hlsl");
}

UStaticMeshComponent::~UStaticMeshComponent()
{

}

void UStaticMeshComponent::Render(URenderer* Renderer, const FMatrix& ViewMatrix, const FMatrix& ProjectionMatrix)
{
    if (!StaticMesh)
    {
        return;
    }
    Renderer->UpdateConstantBuffer(GetWorldMatrix(), ViewMatrix, ProjectionMatrix);
    Renderer->PrepareShader(GetMaterial()->GetShader());
    Renderer->DrawIndexedPrimitiveComponent(
        GetStaticMesh(),
        D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
        MaterailSlots
    );
}

void UStaticMeshComponent::SetStaticMesh(const FString& PathFileName)
{
	StaticMesh = FObjManager::LoadObjStaticMesh(PathFileName);
    
    const TArray<FGroupInfo>& GroupInfos = StaticMesh->GetMeshGroupInfo();
    if (MaterailSlots.size() < GroupInfos.size())
    {
        MaterailSlots.resize(GroupInfos.size());
    }

    // MaterailSlots.size()가 GroupInfos.size() 보다 클 수 있기 때문에, GroupInfos.size()로 설정
    for (int i = 0; i < GroupInfos.size(); ++i) 
    {
        if (MaterailSlots[i].bChangedByUser == false)
        {
            MaterailSlots[i].MaterialName = GroupInfos[i].InitialMaterialName;
        }
    }
}

void UStaticMeshComponent::SetMaterialByUser(const uint32 InMaterialSlotIndex, const FString& InMaterialName)
{
    assert((0 <= InMaterialSlotIndex && InMaterialSlotIndex < MaterailSlots.size()) && "out of range InMaterialSlotIndex");

    if (0 <= InMaterialSlotIndex && InMaterialSlotIndex < MaterailSlots.size())
    {
        MaterailSlots[InMaterialSlotIndex].MaterialName = InMaterialName;
        MaterailSlots[InMaterialSlotIndex].bChangedByUser = true;
    }
    else
    {
        UE_LOG("out of range InMaterialSlotIndex: %d", InMaterialSlotIndex);
    }

    assert(MaterailSlots[InMaterialSlotIndex].bChangedByUser == true);
}

UObject* UStaticMeshComponent::Duplicate()
{
    UStaticMeshComponent* DuplicatedComponent = Cast<UStaticMeshComponent>(NewObject(GetClass()));

    // 공통 속성 복사 (Transform, AttachChildren) - 부모 헬퍼 사용
    CopyCommonProperties(DuplicatedComponent);

    // StaticMeshComponent 전용 속성 복사
    DuplicatedComponent->Material = this->Material;
    DuplicatedComponent->StaticMesh = this->StaticMesh;
    DuplicatedComponent->MaterailSlots = this->MaterailSlots;

    DuplicatedComponent->DuplicateSubObjects();
    return DuplicatedComponent;
}

void UStaticMeshComponent::DuplicateSubObjects()
{
    // 부모의 깊은 복사 수행 (AttachChildren 재귀 복제)
    Super_t::DuplicateSubObjects();
}

void UStaticMeshComponent::Serialize(FObjectData* Data)
{
    FStaticMeshComponentData* ComponentData = dynamic_cast<FStaticMeshComponentData*>(Data);
    assert(ComponentData, "UStaticMeshComponent::Serialize got wrong data type.");

    USceneComponent::Serialize(Data);
    
    if (StaticMesh)
    {
        ComponentData->StaticMesh = StaticMesh->GetAssetPathFileName();
        UE_LOG("SaveScene: StaticMeshComponent StaticMesh saved: %s", ComponentData->StaticMesh.c_str());
    }
    else
    {
        UE_LOG("SaveScene: StaticMeshComponent has no StaticMesh assigned");
    }
    // TODO: Materials 수집
}

void UStaticMeshComponent::DeSerialize(FObjectData* Data)
{
    FStaticMeshComponentData* ComponentData = dynamic_cast<FStaticMeshComponentData*>(Data);
    assert(ComponentData, "UStaticMeshComponent::DeSerialize got wrong data type.");

    USceneComponent::DeSerialize(Data);

    if (!ComponentData->StaticMesh.empty())
    {
        SetStaticMesh(ComponentData->StaticMesh);
    }
    // TODO: Materials 복원
}
