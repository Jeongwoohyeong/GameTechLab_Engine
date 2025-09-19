#include "pch.h"
#include "ObjParser.h"
#include <regex>

IMPLEMENT_SINGLETON(FObjParser)

FObjParser::FObjParser()
{
}

FObjParser::~FObjParser()
{
}

FStaticMesh* FObjParser::LoadObjStaticMesh(const FString& filepath)
{
	ifstream fileStream(filepath);

	if (!fileStream.is_open())
	{
		UE_LOG("파일 열기 실패");
		return nullptr;
	}

	FObjInfo newObjRawData = {};
	TArray<FString> faceLines = {};

	FString line;
	while (std::getline(fileStream, line))
	{
		// # 주석 제외
		size_t commentPos = line.find('#');
		if (commentPos != FString::npos)
		{
			line = line.substr(0, commentPos);
		}

		std::stringstream ss(line);
		FString lineType = {};
		ss >> lineType;

		if (lineType == "v")
		{
			float x, y, z;
			ss >> x >> y >> z;
			// obj 파일은 오른손 좌표계, 왼손 좌표계로 변환하기 위해서는 축 하나를 반전 시켜야함
			//newObjRawData.Position.Emplace(FObjParser::PositionToUEBasis(FVector(x, y, z)));
			newObjRawData.Position.Emplace(FVector(x, y, z));
		}
		else if (lineType == "vt")
		{
			float u, v;
			ss >> u >> v;
			// obj : 왼쪽 아래 (0, 0)
			// UE : 왼쪽 위 (0, 0)
			newObjRawData.Tex.Emplace(FObjParser::UVToBasis(FVector2(u, v)));
		}
		else if (lineType == "vn")
		{
			float x, y, z;
			ss >> x >> y >> z;
			//newObjRawData.Normal.Emplace(FObjParser::PositionToUEBasis(FVector(x, y, z)));
			newObjRawData.Normal.Emplace(FVector(x, y, z));
		}
		else if (lineType == "f")
		{
			faceLines.Emplace(FString(line));
		}
	}

	FStaticMesh* newStaticMesh = new FStaticMesh();
	newStaticMesh->PathFileName = filepath;

	FIndex indices;
	for (const auto& faceLine : faceLines)
	{
		std::stringstream ss(faceLine);
		FString token;
		ss >> token;

		for (size_t i = 0; i < 3; i++)
		{
			ss >> token;
			std::stringstream faceSS(token);
			FString indexStr;

			int vIndex = -1;
			int uvIndex = -1;
			int normIndex = -1;

			std::getline(faceSS, indexStr, '/');
			vIndex = std::stoi(indexStr) - 1;

			std::getline(faceSS, indexStr, '/');
			uvIndex = std::stoi(indexStr) - 1;

			std::getline(faceSS, indexStr, '/');
			normIndex = std::stoi(indexStr) - 1;

			indices.VertexIndices.Emplace(vIndex);
			indices.UVIndices.Emplace(vIndex);
			indices.NormalIndices.Emplace(vIndex);
		}
	}

	size_t size = 0;
	if (newObjRawData.Normal.size() < newObjRawData.Position.size())
	{
		size = newObjRawData.Position.size() <= newObjRawData.Tex.size() ? newObjRawData.Tex.size() : newObjRawData.Position.size();
	}
	else
	{
		size = newObjRawData.Normal.size() <= newObjRawData.Tex.size() ? newObjRawData.Tex.size() : newObjRawData.Normal.size();
	}

	newStaticMesh->Vertices.resize(size);

	for (size_t i = 0;i < size;i++)
	{
		FVector vertex{ -9999.9f, -9999.9f, -9999.9f };
		FVector normal{ -9999.9f, -9999.9f, -9999.9f };
		FVector2 tex{ -9999.9f, -9999.9f };
		FVector4 color{ 0.25f, 0.25f, 0.25f, 1.0f };
		if (i < newObjRawData.Position.size())
		{
			vertex = { newObjRawData.Position[i].X, newObjRawData.Position[i].Y, newObjRawData.Position[i].Z };
		}

		if (i < newObjRawData.Normal.size())
		{
			normal = { newObjRawData.Normal[i].X, newObjRawData.Normal[i].Y, newObjRawData.Normal[i].Z };
		}

		if (i < newObjRawData.Tex.size())
		{
			tex = { newObjRawData.Tex[i].X, newObjRawData.Tex[i].Y };
		}

		newStaticMesh->Vertices[i] = FNormalVertex(vertex, normal, color, tex);
	}

	for (const auto& vIdx : indices.VertexIndices)
	{
		newStaticMesh->Indices.VertexIndices.Emplace(vIdx);
	}
	for (const auto& uvIdx : indices.UVIndices)
	{
		newStaticMesh->Indices.UVIndices.Emplace(uvIdx);
	}
	for (const auto& normIdx : indices.NormalIndices)
	{
		newStaticMesh->Indices.NormalIndices.Emplace(normIdx);
	}

	newStaticMesh->VertexIndexNum = newStaticMesh->Indices.VertexIndices.size();
	newStaticMesh->UVIndexNum = newStaticMesh->Indices.UVIndices.size();
	newStaticMesh->NormalIndexNum = newStaticMesh->Indices.NormalIndices.size();

	if(!newStaticMesh)
	{
		UE_LOG("dasdsa");
	}

	UE_LOG("%s", newStaticMesh->PathFileName.c_str());

	/*for (const auto& e : newStaticMesh->Vertices)
	{
		UE_LOG("pos %f %f %f", e.Position.X, e.Position.Y, e.Position.Z);
		UE_LOG("norm %f %f %f", e.Normal.X, e.Normal.Y, e.Normal.Z);
		UE_LOG("tex %f %f", e.Tex.X, e.Tex.Y);
		UE_LOG("col %f %f %f %f", e.Color.X, e.Color.Y, e.Color.Z, e.Color.W);

	}*/

	return newStaticMesh;
}

FVector FObjParser::PositionToUEBasis(const FVector& InVector)
{
	return FVector(InVector.X, -InVector.Y, InVector.Z);
}

FVector2 FObjParser::UVToBasis(const FVector2& InVector)
{
	return FVector2(InVector.X, 1.0f - InVector.Y);
}
