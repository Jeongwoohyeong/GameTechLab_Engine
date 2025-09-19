#include "pch.h"
#include "ObjParser.h"

IMPLEMENT_SINGLETON(FObjParser)

FObjParser::FObjParser()
{}

FObjParser::~FObjParser()
{}

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
			newObjRawData.Position.Emplace(FObjParser::PositionToUEBasis(FVector(x, y, z)));
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
			newObjRawData.Normal.Emplace(FObjParser::PositionToUEBasis(FVector(x, y, z)));
		}
		else if (lineType == "f")
		{
			faceLines.Emplace(FString(line));
		}
	}

	FStaticMesh* newStaticMesh = new FStaticMesh;
	newStaticMesh->PathFileName = filepath;

	for (const auto& faceLine : faceLines)
	{
		std::stringstream ss(faceLine);
		FString lineType;
		ss >> lineType;

		TArray<FString> tokens;
		FString token;
		while (ss >> token)
		{
			tokens.Emplace(token);
		}

		for (size_t i = 0; i < tokens.size() - 2; i++)
		{
			auto faceParser = [](const FString& s) -> TArray<uint32> {
				std::stringstream ssToken(s);
				FString token;
				TArray<uint32> indices;
				while (std::getline(ssToken, token, '/'))
				{
					if (!token.empty())
					{
						indices.Emplace(std::stoi(token));
					}
					else
					{
						indices.Emplace(0);
					}
				}
				while (indices.size() < 3)
				{
					indices.Emplace(0);
				}

				return indices;
				};

			TArray<uint32> vIndex1 = faceParser(tokens[0]);
			TArray<uint32> vIndex2 = faceParser(tokens[i]);
			TArray<uint32> vIndex3 = faceParser(tokens[i + 1]);

			FVector4 Color = { 0.2f, 0.2f, 0.2f, 1.0f };
			FNormalVertex vertex1, vertex2, vertex3;

			vertex1.Position = (vIndex1[0] > 0) ? newObjRawData.Position[vIndex1[0] - 1] : FVector(0.0f, 0.0f, 0.0f);
			vertex1.Tex = (vIndex1[1] > 0) ? newObjRawData.Tex[vIndex1[1] - 1] : FVector2(0.0f, 0.0f);
			vertex1.Normal = (vIndex1[2] > 0) ? newObjRawData.Normal[vIndex1[2] - 1] : FVector(0.0f, 0.0f, 0.0f);
			vertex1.Color = Color;

			vertex2.Position = (vIndex2[0] > 0) ? newObjRawData.Position[vIndex2[0] - 1] : FVector(0.0f, 0.0f, 0.0f);
			vertex2.Tex = (vIndex2[1] > 0) ? newObjRawData.Tex[vIndex2[1] - 1] : FVector2(0.0f, 0.0f);
			vertex2.Normal = (vIndex2[2] > 0) ? newObjRawData.Normal[vIndex2[2] - 1] : FVector(0.0f, 0.0f, 0.0f);
			vertex2.Color = Color;

			vertex3.Position = (vIndex3[0] > 0) ? newObjRawData.Position[vIndex3[0] - 1] : FVector(0.0f, 0.0f, 0.0f);
			vertex3.Tex = (vIndex3[1] > 0) ? newObjRawData.Tex[vIndex3[1] - 1] : FVector2(0.0f, 0.0f);
			vertex3.Normal = (vIndex3[2] > 0) ? newObjRawData.Normal[vIndex3[2] - 1] : FVector(0.0f, 0.0f, 0.0f);
			vertex3.Color = Color;

			// 오른손 좌표계 -> 왼손 좌표계로 winding 변경
			newStaticMesh->Vertices.Emplace(vertex1);
			newStaticMesh->Vertices.Emplace(vertex3);
			newStaticMesh->Vertices.Emplace(vertex2);

			newStaticMesh->Indices.Emplace(newStaticMesh->Vertices.size() - 3);
			newStaticMesh->Indices.Emplace(newStaticMesh->Vertices.size() - 2);
			newStaticMesh->Indices.Emplace(newStaticMesh->Vertices.size() - 1);
		}
	}

	newStaticMesh->IndexNum = newStaticMesh->Indices.size();

	if(!newStaticMesh)
	{
		UE_LOG("dasdsa");
	}

	UE_LOG("%s", newStaticMesh->PathFileName);	

	for (const auto& e : newStaticMesh->Vertices)
	{
		UE_LOG("pos %f %f %f", e.Position.X, e.Position.Y, e.Position.Z);
		UE_LOG("norm %f %f %f", e.Normal.X, e.Normal.Y, e.Normal.Z);
		UE_LOG("tex %f %f", e.Tex.X, e.Tex.Y);
		UE_LOG("col %f %f %f %f", e.Color.X, e.Color.Y, e.Color.Z, e.Color.W);

	}

	for (const auto& e : newStaticMesh->Indices)
	{
		UE_LOG("index %d ", e);
	}
	UE_LOG("%d", newStaticMesh->IndexNum);

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
