//#pragma once
//#include<d3d11.h>
//
//#include "FVertexStruct.h"
//#include "UMesh.h"
//
//#include "Cone.h"
//#include "Cube.h"
//#include "Cylinder.h"
//#include "Sphere.h"
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
//	ID3D11Buffer* vertexBufferCube;
//	ID3D11Buffer* vertexBufferCylinder;
//	ID3D11Buffer* vertexBufferCone;
//	ID3D11Buffer* vertexBufferSphere;
//
//	// 인덱스 버퍼 생성
//	ID3D11Buffer* indexBufferCube;
//	ID3D11Buffer* indexBufferCylinder;
//	ID3D11Buffer* indexBufferCone;
//
//public :
//	void Initialize(UMesh mesh)
//	{
//		numVerticesCube = sizeof(GCubeVertices) / sizeof(FVertexSimple);
//		numVerticesCylinder = sizeof(GCylinderVertices) / sizeof(FVertexSimple);
//		numVerticesCone = sizeof(GConeVertices) / sizeof(FVertexSimple);
//		numVerticesSphere = sizeof(GSphereVertices) / sizeof(FVertexSimple);
//
//		 mesh.CreateVertexBuffer(vertexBufferCube, GCubeVertices, sizeof(GCubeVertices));
//		 mesh.CreateVertexBuffer(vertexBufferCylinder, GCylinderVertices, sizeof(GCylinderVertices));
//		 mesh.CreateVertexBuffer(vertexBufferCone, GConeVertices, sizeof(GConeVertices));
//		 mesh.CreateVertexBuffer(vertexBufferSphere, GSphereVertices, sizeof(GSphereVertices));
//
//		 mesh.CreateIndexBuffer(indexBufferCube, GCubeIndices, sizeof(GCubeIndices));
//		 mesh.CreateIndexBuffer(indexBufferCylinder, GCylinderIndices, sizeof(GCylinderIndices));
//		 mesh.CreateIndexBuffer(indexBufferCone, GConeIndices, sizeof(GConeIndices));
//	}
//};
//
