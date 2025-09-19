#pragma once
#include "Global/Vector.h"
#include "Global/Matrix.h"
#include <cstdint>

struct FViewProjConstants
{
	FViewProjConstants()
	{
		View = FMatrix::Identity;
		Projection = FMatrix::Identity;
		ViewModeIndex = 0;
	}

	FMatrix View;
	FMatrix Projection;
	uint32 ViewModeIndex;
};


struct FVertex
{
    FVector Position;
    FVector4 Color;

	FVertex() : Position{}, Color{} {}

	FVertex(const FVector& pos, const FVector4& col) : Position(pos), Color(col) {}

    // 동등 비교 연산자
    bool operator==(const FVertex& Other) const
    {
        return Position == Other.Position && Color == Other.Color;
    }
};

namespace std
{
	template<>
	struct hash<FVertex>
	{
		size_t operator()(const FVertex& V) const noexcept
		{
			auto HashCombine = [](size_t Seed, size_t Value) -> size_t
			{
				Seed ^= Value + 0x9e3779b97f4a7c15ULL + (Seed << 6) + (Seed >> 2);
				return Seed;
			};

			size_t H = 0;
			H = HashCombine(H, std::hash<float>{}(V.Position.X));
			H = HashCombine(H, std::hash<float>{}(V.Position.Y));
			H = HashCombine(H, std::hash<float>{}(V.Position.Z));
			H = HashCombine(H, std::hash<float>{}(V.Color.X));
			H = HashCombine(H, std::hash<float>{}(V.Color.Y));
			H = HashCombine(H, std::hash<float>{}(V.Color.Z));
			H = HashCombine(H, std::hash<float>{}(V.Color.W));
			return H;
		}
	};
}

struct FTextVertex
{
	FVector Position;
	FVector2 BaseUv;
};

struct FTextInstance
{
	FVector4 Color;
	FVector Offset;
	uint32 CharId;
};

struct FCharacterInfo
{
	FVector2 UvOffset;
	FVector2 UvSize;
};

struct FNormalVertex
{
	FVector Position;
	FVector Normal;
	FVector4 Color;
	FVector2 Tex;

	FNormalVertex()
		: Position(FVector(-1.0f,-1.0f,-1.0f)), Normal(FVector(-1.0f, -1.0f, -1.0f)),
		Color(FVector4(-1.0f, -1.0f, -1.0f, 1.0f)), Tex(FVector2(-1.0f, -1.0f)) { }
	FNormalVertex(const FVector& v, const FVector& norm, const FVector4& c, const FVector2& uv)
		: Position(v), Normal(norm), Color(c), Tex(uv) {}
};

struct FObjInfo
{
	// 당장 필요한 것만 추가함

	// Vertex
	TArray<FVector> Position;
	TArray<FVector> Normal;
	TArray<FVector> Color; // Material 사용 시 필요할듯
	TArray<FVector2> Tex;

	FObjInfo()
		: Position{}, Normal{}, Color{}, Tex{} {}		
};

struct FIndex
{
	TArray<uint32> VertexIndices;
	TArray<uint32> UVIndices;
	TArray<uint32> NormalIndices;

	FIndex() : VertexIndices{}, UVIndices{}, NormalIndices{} {}

	FIndex(TArray<uint32> vIndex, TArray<uint32> uvIndex, TArray<uint32> normalIndex)
		: VertexIndices(vIndex), UVIndices(uvIndex), NormalIndices(normalIndex) { }
};

// Cooked Data
struct FStaticMesh
{
	FString PathFileName;

	TArray<FNormalVertex> Vertices;
	FIndex Indices;
	uint32 VertexIndexNum;
	uint32 UVIndexNum;
	uint32 NormalIndexNum;

	FStaticMesh() : PathFileName{}, Vertices{}, Indices{},
		VertexIndexNum(0), UVIndexNum(0), NormalIndexNum(0) {}

	FStaticMesh(const FString name, const TArray<FNormalVertex> vertices, const FIndex indices,
		uint32 vIdxNum = 0, uint32 uvIdxNum = 0, uint32 normIdxNum = 0)
		: PathFileName(name), Vertices(vertices), Indices(indices),
		VertexIndexNum(vIdxNum), UVIndexNum(uvIdxNum), NormalIndexNum(normIdxNum) {	}
};

//TMap<char, FCharacterInfo> CharInfoMap;

struct FRay
{
	FVector4 Origin;
	FVector4 Direction;
};

/**
 * @brief Component Type Enum
 */
enum class EComponentType : uint8_t
{
	None = 0,

	Actor,
		//ActorComponent Dervied Type

	Scene,
		//SceneComponent Dervied Type

	Primitive,

	Text,
		//PrimitiveComponent Derived Type

	End = 0xFF
};

/**
 * @brief UObject Primitive Type Enum
 */
enum class EPrimitiveType : uint8_t
{
	None = 0,
	Sphere,
	Cube,
	Triangle,
	Square,
	Torus,
	Arrow,
	CubeArrow,
	Ring,
	Line,

	End = 0xFF
};

/**
 * @brief RasterizerState Enum
 */
enum class ECullMode : uint8_t
{
	Back,
	Front,
	None,

	End = 0xFF
};

enum class EFillMode : uint8_t
{
	WireFrame,
	Solid,

	End = 0xFF
};

/**
 * @brief View Mode State
 */
enum class EViewModeIndex : uint32
{
	Lit,
	Unlit,
	Wireframe,

	End
};

enum class ESamplerType
{
	Text,
};

/**
 * @brief Engine Show Flags for controlling visibility of various elements
 */
enum class EEngineShowFlags : uint64
{
	SF_None = 0,
	SF_Primitives = 1 << 0,      // Show primitive objects
	SF_BillboardText = 1 << 1,   // Show billboard text
	SF_Grid = 1 << 2,            // Show grid
	SF_Bounds = 1 << 3,          // Show bounding boxes

	// Default flags (everything visible)
	SF_Default = SF_Primitives | SF_BillboardText | SF_Grid
};

// Bitwise operators for EEngineShowFlags
inline EEngineShowFlags operator|(EEngineShowFlags a, EEngineShowFlags b)
{
	return static_cast<EEngineShowFlags>(static_cast<uint64>(a) | static_cast<uint64>(b));
}

inline EEngineShowFlags operator&(EEngineShowFlags a, EEngineShowFlags b)
{
	return static_cast<EEngineShowFlags>(static_cast<uint64>(a) & static_cast<uint64>(b));
}

inline EEngineShowFlags operator^(EEngineShowFlags a, EEngineShowFlags b)
{
	return static_cast<EEngineShowFlags>(static_cast<uint64>(a) ^ static_cast<uint64>(b));
}

inline EEngineShowFlags operator~(EEngineShowFlags a)
{
	return static_cast<EEngineShowFlags>(~static_cast<uint64>(a));
}

inline EEngineShowFlags& operator|=(EEngineShowFlags& a, EEngineShowFlags b)
{
	return a = a | b;
}

inline EEngineShowFlags& operator&=(EEngineShowFlags& a, EEngineShowFlags b)
{
	return a = a & b;
}

inline EEngineShowFlags& operator^=(EEngineShowFlags& a, EEngineShowFlags b)
{
	return a = a ^ b;
}

inline bool HasFlag(EEngineShowFlags flags, EEngineShowFlags flag)
{
	return (flags & flag) == flag;
}

/**
 * @brief Render State Settings for Actor's Component
 */
struct FRenderState
{
	ECullMode CullMode = ECullMode::None;
	EFillMode FillMode = EFillMode::Solid;
};
