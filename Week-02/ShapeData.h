#pragma once

enum EShape {
	Sphere,
	Cube,
	Cone,
	Cylinder,
	Line
};


//#include<d3d11.h>
//#include "FVertexStruct.h"
//#include "Cube.h"
//#include "URenderer.h"
//
//class ShapeData
//{
//private:
//	UINT numVerticesCube;
//	UINT numVerticesCylinder;
//	UINT numVerticesCone;
//	UINT numVerticesSphere;
//
//	// 정점 버퍼 생성
//	static ID3D11Buffer* vertexBufferCube;
//	//static ID3D11Buffer* vertexBufferCylinder;
//	//static ID3D11Buffer* vertexBufferCone;
//	//static ID3D11Buffer* vertexBufferSphere;
//
//	// 인덱스 버퍼 생성
//	static ID3D11Buffer* indexBufferCube;
//	//static ID3D11Buffer* indexBufferCylinder;
//	//static ID3D11Buffer* indexBufferCone;
//
//public :
//	void Initialize(URenderer* renderer)
//	{
//		numVerticesCube = sizeof(GCubeVertices) / sizeof(FVertexSimple);
//		renderer->CreateVertexBuffer(&vertexBufferCube, GCubeVertices, sizeof(GCubeVertices));
//		renderer->CreateIndexBuffer(&indexBufferCube, GCubeIndices, sizeof(GCubeIndices));
//	}
//};
//

//