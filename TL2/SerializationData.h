#pragma once

// ========================================
// Version 1 (Legacy - 하위 호환용)
// ========================================
struct FPrimitiveData
{
    uint32 UUID = 0;
    FVector Location;
    FVector Rotation;
    FVector Scale;
    FString Type;
    FString ObjStaticMeshAsset;
};

// ========================================
// Version 2 (Component Hierarchy Support)
// ========================================
// FObjectData를 상속하는 Class들은 굳이 UUID를 초기화하지 않아도 된다.
// Serialization, DeSerialization 시 필요하다고 판단될 때에만 초기화한다.
struct FObjectData
{
    uint32 UUID = 0;
    virtual ~FObjectData() = default;
};

struct FComponentData : public FObjectData
{
    uint32 OwnerActorUUID = 0;
    uint32 ParentComponentUUID = 0;  // 0이면 RootComponent (부모 없음)
    FString Type;  // "StaticMeshComponent", "AABoundingBoxComponent" 등

    // Transform
    FVector RelativeLocation;
    FVector RelativeRotation;
    FVector RelativeScale;

    TArray<FString> Materials;
};

struct FStaticMeshComponentData : public FComponentData
{
    FString StaticMesh;
};

struct FBillboardComponentData : public FComponentData
{
    FString Texture;
};

struct FTextComponentData : public FComponentData
{
    FString Text;
};

struct FDecalComponentData : public FComponentData
{
    FString Texture;  // Asset path

    // Fade Property
    uint8 FadeType = 0;
    float Duration = 0.0f;
    float Min = 0.0f;
    float Max = 0.0f;
    float Alpha = 0.0f;
    bool bIsFadeEnabled = false;
    bool bIsFadeStart = false;
    bool bIsLoop = false;
    
    float ElapsedTime = 0.0f;
};

struct FActorData : public FObjectData
{
    FString Type;  // "StaticMeshActor" 등
    FString Name;
    uint32 RootComponentUUID = 0;
};

struct FPerspectiveCameraData : public FObjectData
{
    FVector Location;
    FVector Rotation;
    float FOV;
    float NearClip;
    float FarClip;
};

struct FSceneData : public FObjectData
{
    uint32 Version = 2;
    uint32 NextUUID = 0;
    TArray<FActorData> Actors;
    TArray<FComponentData *> Components;
    FPerspectiveCameraData Camera;
};