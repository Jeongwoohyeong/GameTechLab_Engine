#pragma once
#include "Gizmo.h"
#include <vector>

class WorldGizmo : public Gizmo
{
public:
    std::vector<FVertexSimple>      xzGridVertices;
    std::vector<unsigned int>       xzGridIndices;

    ID3D11Buffer* gridVerticesBuffer;
    ID3D11Buffer* gridIndicesBuffer;

    // 색상 프리셋 (좌표 xyz는 0,0,0으로 두고 색만 사용)
    FVertexSimple Minor = { 0,0,0,  0.55f,0.55f,0.55f,1.0f }; // 연회색
    FVertexSimple Major = { 0,0,0,  0.30f,0.30f,0.30f,1.0f }; // 진회색
    FVertexSimple XCol = { 0,0,0,  1.00f,0.20f,0.20f,1.0f }; // X축 빨강
    FVertexSimple ZCol = { 0,0,0,  0.20f,0.40f,1.00f,1.0f }; // Z축 파랑
    FVertexSimple YCol = { 0,0,0,  0.40f,1.00f,0.40f,1.0f }; // Y축 초록

    void Initialize()
    {
        // XZ 평면 그리드 + Y축 기즈모
        BuildGridXZwithYaxis(
            1.0f,       // Spacing
            100,         // HalfCount
            10,          // MajorEvery
            0.0f,       // YLevel
            xzGridVertices,
            xzGridIndices,
            Minor,
            Major,
            XCol,
            ZCol,
            YCol);
        gridVerticesBuffer = nullptr;
        gridIndicesBuffer = nullptr;
	}

    void Render(UMesh mesh)
    {
        mesh.CreateVertexBuffer(gridVerticesBuffer, xzGridVertices.data(), xzGridVertices.size() * sizeof(FVertexSimple));
        mesh.CreateIndexBuffer(gridIndicesBuffer, xzGridIndices.data(), xzGridIndices.size() * sizeof(UINT));


        mesh.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        renderer.UpdateConstant(Camera::GetInstance().MakeMVP(FMatrix::MakeWorld({ 1.0f, 1.0f, 1.0f }, { ToRadian(0.0f), ToRadian(0.0f), ToRadian(0.0f) }, { 0.0f, 0.0f, 0.0f })));
        renderer.RenderPrimitive(xzGridVertices, xzGridVertices.size(), ib, xzGridIndices.size());
    }
private:
    void BuildGridXZwithYaxis(
        float Spacing,
        int   HalfCount,
        int   MajorEvery,
        float YLevel,
        std::vector<FVertexSimple>& outVertices,
        std::vector<unsigned int>& outIndices,
        const FVertexSimple& MinorColor,   // 보통 연한 회색
        const FVertexSimple& MajorColor,   // 진한 회색
        const FVertexSimple& AxisXColor,   // X축(빨강)
        const FVertexSimple& AxisZColor,   // Z축(파랑)
        const FVertexSimple& AxisYColor)   // Y축(초록)
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

        // Z 방향 평행선 (고정 X에서 z-축으로 뻗는 선)
        for (int i = -HalfCount; i <= HalfCount; ++i)
        {
            float x = i * Spacing;
            bool isAxisX = (i == 0);
            bool isMajor = (i % MajorEvery == 0);

            const FVertexSimple& col = isAxisX ? AxisXColor : (isMajor ? MajorColor : MinorColor);
            pushLine(x, minZ, x, maxZ, col);
        }

        // X 방향 평행선 (고정 Z에서 x-축으로 뻗는 선)
        for (int k = -HalfCount; k <= HalfCount; ++k)
        {
            float z = k * Spacing;
            bool isAxisZ = (k == 0);
            bool isMajor = (k % MajorEvery == 0);

            const FVertexSimple& col = isAxisZ ? AxisZColor : (isMajor ? MajorColor : MinorColor);
            pushLine(minX, z, maxX, z, col);
        }

        // Y축 기즈모 (원점 기준 위/아래로 뻗는 선)
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
};
