#pragma once
#include "Core/Object.h"
#include "Math/AABB.h"

class UMaterial;

class UStaticMesh : public UObject
{
	DECLARE_CLASS(UStaticMesh, UObject)

public:
	UStaticMesh();
	UStaticMesh(FStaticMesh* InStaticMeshAsset);
	~UStaticMesh();

	const FString& GetAssetPathFileName() const { return StaticMeshAsset->PathFileName; }
	ID3D11Buffer* GetVertexBuffer() { return VertexBuffer; }
	ID3D11Buffer* GetIndexBuffer() { return IndexBuffer; }
	uint32 GetIndexNum() { return IndexNum; }
	FStaticMesh* GetStaticMeshAsset();
	FAABB GetLocalAABB();
	EPrimitiveType GetPrimitiveType() const { return PrimitiveType; }
	const FObjMaterialInfo* GetMaterialInfo() const;
	const FString& GetKdTextureFilePath() const;
	const FString& GetKsTextureFilePath() const;
	const FString& GetBumpTextureFilePath() const;

	void SetStaticMeshAsset(FStaticMesh* InStaticMeshAsset);
	void SetPrimtiveType(EPrimitiveType Type) { PrimitiveType = Type; }
	void SetMaterialInfo(FObjMaterialInfo* InMaterialInfo);


private:
	void CalculateLocalAABB();

	FStaticMesh* StaticMeshAsset = nullptr;
	UMaterial* MaterialInfo = nullptr;
	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;
	uint32 IndexNum = 0;
	EPrimitiveType PrimitiveType = EPrimitiveType::None;
	FAABB AABB = FAABB();
};
