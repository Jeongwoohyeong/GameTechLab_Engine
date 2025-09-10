#include "WorldGizmo.h"
#include "URenderer.h"
#include "UCamera.h"

void WorldGizmo::Initialize(URenderer* renderer)
{
	ParentTransform = new FTransform();
	ParentTransform->SetLocation(FVector(0.0f, 0.0f, 0.0f));
	ParentTransform->SetScale(FVector(1.0f, 1.0f, 1.0f));
    ParentTransform->LoadRotaion(FVector(0.0f, 0.0f, 0.0f));
	ParentTransform->LoadQuaternion(FQuaternion());

    // XZ 평면 그리드 + Y축 기즈모 생성
    BuildGridXZwithYaxis(
        1.0f,   // Spacing
        100,    // HalfCount
        10,     // MajorEvery
        0.0f,   // YLevel
        xzGridVertices,
        xzGridIndices,
        Minor,
        Major,
        YColor,
        XColor,
        ZColor);

    gridVerticesBuffer = nullptr;
    gridIndicesBuffer = nullptr;

    renderer->CreateVertexBuffer(&gridVerticesBuffer,
        xzGridVertices.data(),
        static_cast<unsigned int>(xzGridVertices.size() * sizeof(FVertexSimple)));

    renderer->CreateIndexBuffer(&gridIndicesBuffer,
        xzGridIndices.data(),
        static_cast<unsigned int>(xzGridIndices.size() * sizeof(unsigned int)));
}

void WorldGizmo::Render(URenderer* renderer)
{
    renderer->SetTopology(true); // 선으로 렌더링
    renderer->UpdateConstant(UCamera::GetInstance().MakeMVP(ParentTransform->GetTransformMatrix()));
    renderer->RenderMesh(gridVerticesBuffer,
        static_cast<unsigned int>(xzGridVertices.size()),
        gridIndicesBuffer,
        static_cast<unsigned int>(xzGridIndices.size()),
        sizeof(FVertexSimple));
    renderer->SetTopology(false); //삼각형으로 렌더링
}

void WorldGizmo::Release()
{
    if (gridVerticesBuffer)
    {
        gridVerticesBuffer->Release();
        gridVerticesBuffer = nullptr;
    }
    if (gridIndicesBuffer)
    {
        gridIndicesBuffer->Release();
        gridIndicesBuffer = nullptr;
	}
}

void WorldGizmo::BuildGridXZwithYaxis(
    float Spacing,
    int   HalfCount,
    int   MajorEvery,
    float YLevel,
    std::vector<FVertexSimple>& outVertices,
    std::vector<unsigned int>& outIndices,
    const FVertexSimple& MinorColor,
    const FVertexSimple& MajorColor,
    const FVertexSimple& AxisXColor,
    const FVertexSimple& AxisZColor,
    const FVertexSimple& AxisYColor)
{
    outVertices.clear();
    outIndices.clear();

    auto pushLine = [&](float x0, float z0, float x1, float z1, const FVertexSimple& c)
        {
            unsigned int base = static_cast<unsigned int>(outVertices.size());

            FVertexSimple v0 = { x0, YLevel, z0, c.r, c.g, c.b, c.a };
            FVertexSimple v1 = { x1, YLevel, z1, c.r, c.g, c.b, c.a };

            outVertices.push_back(v0);
            outVertices.push_back(v1);
            outIndices.push_back(base + 0);
            outIndices.push_back(base + 1);
        };

    const float minX = -HalfCount * Spacing;
    const float maxX = HalfCount * Spacing;
    const float minZ = -HalfCount * Spacing;
    const float maxZ = HalfCount * Spacing;

    // Z 방향 라인
    for (int i = -HalfCount; i <= HalfCount; ++i)
    {
        float x = i * Spacing;
        bool isAxisX = (i == 0);
        bool isMajor = (i % MajorEvery == 0);

        const FVertexSimple& col = isAxisX ? AxisXColor : (isMajor ? MajorColor : MinorColor);
        pushLine(x, minZ, x, maxZ, col);
    }

    // X 방향 라인
    for (int k = -HalfCount; k <= HalfCount; ++k)
    {
        float z = k * Spacing;
        bool isAxisZ = (k == 0);
        bool isMajor = (k % MajorEvery == 0);

        const FVertexSimple& col = isAxisZ ? AxisZColor : (isMajor ? MajorColor : MinorColor);
        pushLine(minX, z, maxX, z, col);
    }

    // Y축 기즈모
    unsigned int base = static_cast<unsigned int>(outVertices.size());
    FVertexSimple yAxisStart = { 0.0f, YLevel - (HalfCount * Spacing), 0.0f,
                                 AxisYColor.r, AxisYColor.g, AxisYColor.b, AxisYColor.a };
    FVertexSimple yAxisEnd = { 0.0f, YLevel + (HalfCount * Spacing), 0.0f,
                                 AxisYColor.r, AxisYColor.g, AxisYColor.b, AxisYColor.a };

    outVertices.push_back(yAxisStart);
    outVertices.push_back(yAxisEnd);
    outIndices.push_back(base + 0);
    outIndices.push_back(base + 1);
}
