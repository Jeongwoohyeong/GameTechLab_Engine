#pragma once
#include "Core/Object.h"
class UStaticMesh;

class UResourceManager : public UObject
{
	DECLARE_CLASS(UResourceManager, UObject)
	DECLARE_SINGLETON(UResourceManager)

public:
	void Initialize();

	void Release();

	UStaticMesh* GetStaticMesh(const FString& Path);

	TArray<FVertex>* GetVertexData(EPrimitiveType Type);
	ID3D11Buffer* GetVertexBuffer(EPrimitiveType Type);
	uint32 GetVertexNum(EPrimitiveType Type);

	//////////////////리팩토링 예정///////////
	ID3D11Buffer* GetTextVertexBuffer() const { return TextVertexBuffer; }
	uint32 GetTextNumVertices() const { return TextVertexNum; }
	/////////////////////////////////////////

	void CreateTextSampler();
	ID3D11ShaderResourceView* LoadTexture(const FString& Path);
	ID3D11ShaderResourceView* GetTexture(const FString& Path);

	ID3D11SamplerState* GetSamplerState(ESamplerType Type);

	int32 GetCharInfoIdx(WCHAR Char);
	const TArray<FCharacterInfo>& GetCharInfos();

private:
	void LoadCharInfoMap();

	TArray<FString> DefaultAssetPaths = { "Data/cube-tex.obj", "Data/triangle.obj", "Data/square.obj", "Data/sphere.obj", "Data/minion.obj" };
	TMap<FString, FStaticMesh*> StaticMeshAssets;
	TMap<FString, UStaticMesh*> StaticMeshes;

	TMap<FString, ID3D11ShaderResourceView*> ShaderResourceViews;
	TMap<ESamplerType, ID3D11SamplerState*> SamplerStates;







	////////////////////////////////////////////For Gizmo////////////////////////////////////
	TMap<EPrimitiveType, TArray<FVertex>*> VertexData;
	TMap<EPrimitiveType, ID3D11Buffer*> VertexBuffers;
	TMap<EPrimitiveType, uint32> VertexNum;
	////////////////////////////////////////////For Gizmo////////////////////////////////////
	// 
	////////////////////////////////////////////For Text////////////////////////////////////
	TArray<FTextVertex>* TextVertexData;
	ID3D11Buffer* TextVertexBuffer = nullptr;
	uint32 TextVertexNum = 0;

	TArray<FCharacterInfo> CharInfos;
	TMap<WCHAR, int32> CharInfoIdxMap;
	////////////////////////////////////////////For Text////////////////////////////////////

	
};
