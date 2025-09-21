#pragma once
#include "Core/Object.h"

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

	void SetStaticMeshAsset(FStaticMesh* InStaticMeshAsset);

	FStaticMesh* GetStaticMeshAsset();	


private:
	FStaticMesh* StaticMeshAsset = nullptr;
	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;
	uint32 IndexNum = 0;
};
