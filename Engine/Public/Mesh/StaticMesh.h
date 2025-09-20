#pragma once
#include "Core/Object.h"
#include "Math/AABB.h"

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
	EPrimitiveType GetPrimitiveType() const { return PrimitiveType; }

	void SetStaticMeshAsset(FStaticMesh* InStaticMeshAsset);
	FAABB CalculateAABB() const;
	


private:
	FStaticMesh* StaticMeshAsset = nullptr;
	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;
	uint32 IndexNum = 0;
	EPrimitiveType PrimitiveType = EPrimitiveType::None;
};
