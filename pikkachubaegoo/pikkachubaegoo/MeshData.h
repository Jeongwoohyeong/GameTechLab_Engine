#pragma once
#include <vector>
#include "Vertex.h"

using namespace std;

struct FMeshData
{
public:
	static FMeshData SpriteMeshData;
	vector<FVertex> Vertices;
	vector<uint32_t> Indices;
private:
	FMeshData(vector<FVertex> vertices, vector<uint32_t> indices) : Vertices(vertices), Indices(indices) {};

};