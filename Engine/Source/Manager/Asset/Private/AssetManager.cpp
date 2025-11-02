#include "pch.h"
#include "Manager/Asset/Public/AssetManager.h"
#include "Render/Renderer/Public/Renderer.h"
#include "Component/Mesh/Public/VertexDatas.h"
#include "Physics/Public/AABB.h"
#include "Texture/Public/Texture.h"
#include "Manager/Asset/Public/ObjManager.h"
#include "Manager/Path/Public/PathManager.h"
#include "Render/Renderer/Public/RenderResourceFactory.h"

IMPLEMENT_SINGLETON_CLASS(UAssetManager, UObject)
UAssetManager::UAssetManager()
{
	TextureManager = new FTextureManager();
}

UAssetManager::~UAssetManager() = default;

void UAssetManager::Initialize()
{
	TextureManager->LoadAllTexturesFromDirectory(UPathManager::GetInstance().GetDataPath());
	// Data 폴더 속 모든 .obj 파일 로드 및 캐싱
	LoadAllObjStaticMesh();

	VertexDatas.emplace(EPrimitiveType::Torus, &VerticesTorus);
	VertexDatas.emplace(EPrimitiveType::Arrow, &VerticesArrow);
	VertexDatas.emplace(EPrimitiveType::CubeArrow, &VerticesCubeArrow);
	VertexDatas.emplace(EPrimitiveType::Ring, &VerticesRing);
	VertexDatas.emplace(EPrimitiveType::Line, &VerticesLine);
	VertexDatas.emplace(EPrimitiveType::Sprite, &VerticesVerticalSquare);

	IndexDatas.emplace(EPrimitiveType::Sprite, &IndicesVerticalSquare);
	IndexBuffers.emplace(EPrimitiveType::Sprite,
		FRenderResourceFactory::CreateIndexBuffer(IndicesVerticalSquare.data(), static_cast<int>(IndicesVerticalSquare.size()) * sizeof(uint32)));

	NumIndices.emplace(EPrimitiveType::Sprite, static_cast<uint32>(IndicesVerticalSquare.size()));
	
	// TArray.GetData(), TArray.Num()*sizeof(FVertexSimple), TArray.GetTypeSize()
	VertexBuffers.emplace(EPrimitiveType::Torus, FRenderResourceFactory::CreateVertexBuffer(
		VerticesTorus.data(), static_cast<int>(VerticesTorus.size() * sizeof(FNormalVertex))));
	VertexBuffers.emplace(EPrimitiveType::Arrow, FRenderResourceFactory::CreateVertexBuffer(
		VerticesArrow.data(), static_cast<int>(VerticesArrow.size() * sizeof(FNormalVertex))));
	VertexBuffers.emplace(EPrimitiveType::CubeArrow, FRenderResourceFactory::CreateVertexBuffer(
		VerticesCubeArrow.data(), static_cast<int>(VerticesCubeArrow.size() * sizeof(FNormalVertex))));
	VertexBuffers.emplace(EPrimitiveType::Ring, FRenderResourceFactory::CreateVertexBuffer(
		VerticesRing.data(), static_cast<int>(VerticesRing.size() * sizeof(FNormalVertex))));
	VertexBuffers.emplace(EPrimitiveType::Line, FRenderResourceFactory::CreateVertexBuffer(
		VerticesLine.data(), static_cast<int>(VerticesLine.size() * sizeof(FNormalVertex))));
	VertexBuffers.emplace(EPrimitiveType::Sprite, FRenderResourceFactory::CreateVertexBuffer(
		VerticesVerticalSquare.data(), static_cast<int>(VerticesVerticalSquare.size() * sizeof(FNormalVertex))));

	NumVertices.emplace(EPrimitiveType::Torus, static_cast<uint32>(VerticesTorus.size()));
	NumVertices.emplace(EPrimitiveType::Arrow, static_cast<uint32>(VerticesArrow.size()));
	NumVertices.emplace(EPrimitiveType::CubeArrow, static_cast<uint32>(VerticesCubeArrow.size()));
	NumVertices.emplace(EPrimitiveType::Ring, static_cast<uint32>(VerticesRing.size()));
	NumVertices.emplace(EPrimitiveType::Line, static_cast<uint32>(VerticesLine.size()));
	NumVertices.emplace(EPrimitiveType::Sprite, static_cast<uint32>(VerticesVerticalSquare.size()));
	
	// Calculate AABB for all primitive types (excluding StaticMesh)
	for (const auto& Pair : VertexDatas)
	{
		EPrimitiveType Type = Pair.first;
		const auto* Vertices = Pair.second;
		if (!Vertices || Vertices->empty())
			continue;

		AABBs[Type] = CalculateAABB(*Vertices);
	}

	// Calculate AABB for each StaticMesh
	for (const auto& MeshPair : StaticMeshCache)
	{
		const FName& ObjPath = MeshPair.first;
		const auto& Mesh = MeshPair.second;
		if (!Mesh || !Mesh->IsValid())
			continue;

		const auto& Vertices = Mesh->GetVertices();
		if (Vertices.empty())
			continue;

		StaticMeshAABBs[ObjPath] = CalculateAABB(Vertices);
	}
}

void UAssetManager::Release()
{
	// TMap.Value()
	for (auto& Pair : VertexBuffers)
	{
		SafeRelease(Pair.second);
	}
	for (auto& Pair : IndexBuffers)
	{
		SafeRelease(Pair.second);
	}

	for (auto& Pair : StaticMeshVertexBuffers)
	{
		SafeRelease(Pair.second);
	}
	for (auto& Pair : StaticMeshIndexBuffers)
	{
		SafeRelease(Pair.second);
	}

	StaticMeshCache.clear();	// unique ptr 이라서 자동으로 해제됨
	StaticMeshVertexBuffers.clear();
	StaticMeshIndexBuffers.clear();

	// TMap.Empty()
	VertexBuffers.clear();
	IndexBuffers.clear();
	
	SafeDelete(TextureManager);
}

/**
 * @brief Data/ 경로 하위에 모든 .obj 파일을 로드 후 캐싱한다
 */
void UAssetManager::LoadAllObjStaticMesh()
{
	TArray<FName> ObjList;
	const FString DataDirectory = "Data/"; // 검색할 기본 디렉토리
	// 디렉토리가 실제로 존재하는지 먼저 확인합니다.
	if (std::filesystem::exists(DataDirectory) && std::filesystem::is_directory(DataDirectory))
	{
		// recursive_directory_iterator를 사용하여 디렉토리와 모든 하위 디렉토리를 순회합니다.
		for (const auto& Entry : std::filesystem::recursive_directory_iterator(DataDirectory))
		{
			// 현재 항목이 일반 파일이고, 확장자가 ".obj"인지 확인합니다.
			if (Entry.is_regular_file() && Entry.path().extension() == ".obj")
			{
				// .generic_string()을 사용하여 OS에 상관없이 '/' 구분자를 사용하는 경로를 바로 얻습니다.
				FString PathString = Entry.path().generic_string();

				// 찾은 파일 경로를 FName으로 변환하여 ObjList에 추가합니다.
				ObjList.push_back(FName(PathString));
			}
		}
	}

	// Enable winding order flip for this OBJ file
	FObjImporter::Configuration Config;
	Config.bFlipWindingOrder = false;
	Config.bIsBinaryEnabled = true;
	Config.bPositionToUEBasis = true;
	Config.bNormalToUEBasis = true;
	Config.bUVToUEBasis = true;

	// 범위 기반 for문을 사용하여 배열의 모든 요소를 순회합니다.
	for (const FName& ObjPath : ObjList)
	{
		// FObjManager가 UStaticMesh 포인터를 반환한다고 가정합니다.
		// 주의: LoadObjStaticMesh 내부에서 이미 AddStaticMeshToCache를 호출하여
		// StaticMeshCache에 추가하므로 여기서 다시 추가하면 double delete 발생!
		UStaticMesh* LoadedMesh = FObjManager::LoadObjStaticMesh(ObjPath, Config);

		// 로드에 성공했는지 확인합니다.
		if (LoadedMesh)
		{
			// TODO - 올바른 해결법이 아닐지도
			// StaticMeshCache.emplace(ObjPath, LoadedMesh);
			// 위 줄 삭제됨: LoadObjStaticMesh에서 이미 캐시에 추가하므로 중복 방지

			StaticMeshVertexBuffers.emplace(ObjPath, this->CreateVertexBuffer(LoadedMesh->GetVertices()));
			StaticMeshIndexBuffers.emplace(ObjPath, this->CreateIndexBuffer(LoadedMesh->GetIndices()));
		}
	}
}

ID3D11Buffer* UAssetManager::GetVertexBuffer(FName InObjPath)
{
	if (StaticMeshVertexBuffers.count(InObjPath))
	{
		return StaticMeshVertexBuffers[InObjPath];
	}
	return nullptr;
}

ID3D11Buffer* UAssetManager::GetIndexBuffer(FName InObjPath)
{
	if (StaticMeshIndexBuffers.count(InObjPath))
	{
		return StaticMeshIndexBuffers[InObjPath];
	}
	return nullptr;
}

ID3D11Buffer* UAssetManager::CreateVertexBuffer(TArray<FNormalVertex> InVertices)
{
	return FRenderResourceFactory::CreateVertexBuffer(InVertices.data(), static_cast<int>(InVertices.size()) * sizeof(FNormalVertex));
}

ID3D11Buffer* UAssetManager::CreateIndexBuffer(TArray<uint32> InIndices)
{
	return FRenderResourceFactory::CreateIndexBuffer(InIndices.data(), static_cast<int>(InIndices.size()) * sizeof(uint32));
}

TArray<FNormalVertex>* UAssetManager::GetVertexData(EPrimitiveType InType)
{
	return VertexDatas[InType];
}

ID3D11Buffer* UAssetManager::GetVertexbuffer(EPrimitiveType InType)
{
	return VertexBuffers[InType];
}

uint32 UAssetManager::GetNumVertices(EPrimitiveType InType)
{
	return NumVertices[InType];
}

TArray<uint32>* UAssetManager::GetIndexData(EPrimitiveType InType)
{
	return IndexDatas[InType];
}

ID3D11Buffer* UAssetManager::GetIndexBuffer(EPrimitiveType InType)
{
	return IndexBuffers[InType];
}

uint32 UAssetManager::GetNumIndices(EPrimitiveType InType)
{
	return NumIndices[InType];
}

FAABB& UAssetManager::GetAABB(EPrimitiveType InType)
{
	return AABBs[InType];
}

FAABB& UAssetManager::GetStaticMeshAABB(FName InName)
{
	return StaticMeshAABBs[InName];
}

// StaticMesh Cache Accessors
UStaticMesh* UAssetManager::GetStaticMeshFromCache(const FName& InObjPath)
{
	auto It = StaticMeshCache.find(InObjPath);
	if (It != StaticMeshCache.end())
	{
		return It->second.get();
	}
	return nullptr;
}

void UAssetManager::AddStaticMeshToCache(const FName& InObjPath, UStaticMesh* InStaticMesh)
{
	if (!InStaticMesh)
		return;

	if (StaticMeshCache.find(InObjPath) == StaticMeshCache.end())
	{
		StaticMeshCache.emplace(InObjPath, std::unique_ptr<UStaticMesh>(InStaticMesh));
	}
}

/**
 * @brief Vertex 배열로부터 AABB(Axis-Aligned Bounding Box)를 계산하는 헬퍼 함수
 * @param Vertices 정점 데이터 배열
 * @return 계산된 FAABB 객체
 */
FAABB UAssetManager::CalculateAABB(const TArray<FNormalVertex>& Vertices)
{
	FVector MinPoint(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	FVector MaxPoint(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (const auto& Vertex : Vertices)
	{
		MinPoint.X = std::min(MinPoint.X, Vertex.Position.X);
		MinPoint.Y = std::min(MinPoint.Y, Vertex.Position.Y);
		MinPoint.Z = std::min(MinPoint.Z, Vertex.Position.Z);

		MaxPoint.X = std::max(MaxPoint.X, Vertex.Position.X);
		MaxPoint.Y = std::max(MaxPoint.Y, Vertex.Position.Y);
		MaxPoint.Z = std::max(MaxPoint.Z, Vertex.Position.Z);
	}

	return FAABB(MinPoint, MaxPoint);
}

/**
 * @brief 넘겨준 경로로 캐싱된 UTexture 포인터를 반환해주는 함수
 * @param 로드할 텍스처 경로
 * @return 캐싱된 UTexture 포인터
 */
UTexture* UAssetManager::LoadTexture(const FName& InFilePath)
{
	return TextureManager->LoadTexture(InFilePath);
}

/**
 * @brief 지금까지 캐싱된 UTexture 포인터 목록 반환해주는 함수
 * @return {경로, 캐싱된 UTexture 포인터}
 */
const TMap<FName, UTexture*>& UAssetManager::GetTextureCache() const
{
	return TextureManager->GetTextureCache();
}
