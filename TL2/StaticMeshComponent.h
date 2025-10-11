#pragma once
#include "MeshComponent.h"
#include "Enums.h"
#include "StaticMesh.h"

class UStaticMesh;
class UShader;
class UTexture;
struct FPrimitiveData;

struct FMaterialSlot
{
    FString MaterialName;
    bool bChangedByUser = false; // user에 의해 직접 Material이 바뀐 적이 있는지.
};

class UStaticMeshComponent : public UMeshComponent
{
public:
    DECLARE_CLASS(UStaticMeshComponent, UMeshComponent)
    UStaticMeshComponent();

protected:
    ~UStaticMeshComponent() override;

public:
    void Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj) override;

    void SetStaticMesh(const FString& PathFileName);
    UStaticMesh* GetStaticMesh() const { return StaticMesh; }

    void SetMaterialByUser(const uint32 InMaterialSlotIndex, const FString& InMaterialName);

    const TArray<FMaterialSlot>& GetMaterailSlots() const { return MaterailSlots; }

    UObject* Duplicate() override;
    void DuplicateSubObjects() override;

    void Serialize(FObjectData* Data) override;
    void DeSerialize(FObjectData* Data) override;

protected:
    // [PIE] 주소 복사 / NOTE: 만약 복사 후에도 GPU 버퍼 내용을 다르게 갖고 싶은 경우 깊은 복사를 해서 버퍼를 2개 생성하는 방법도 고려
    UStaticMesh* StaticMesh = nullptr;
    // [PIE] 값 복사 (배열 전체 값 복사)
    TArray<FMaterialSlot> MaterailSlots;
};

