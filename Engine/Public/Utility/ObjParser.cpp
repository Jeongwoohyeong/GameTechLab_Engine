#include "pch.h"
#include "ObjParser.h"
#include "Manager/Path/PathManager.h"

IMPLEMENT_SINGLETON(FObjParser)

FObjParser::FObjParser()
{
}

FObjParser::~FObjParser()
{
}

FStaticMesh* FObjParser::LoadObjStaticMesh(const FString& filePath)
{	
	ifstream objFile(filePath);

	if (!objFile.is_open())
	{
		UE_LOG("파일 열기 실패");
		return nullptr;
	}

	FObjInfo newObjRawData = {};	
	FString line;

	FVector tempVec;
	FVector2 tempVec2;

	FStaticMesh* newStaticMesh = new FStaticMesh();
	newStaticMesh->PathFileName = filePath;

	while (std::getline(objFile, line))
	{
		std::stringstream ss(line);
		FString token;

		// 공백라인이거나 주석 생략
		if (line.empty() || line[0] == '#')
		{
			continue;
		}

		// 라인의 첫 두 문자 검사 - vertex
		if (line.substr(0, 2) == "v ")
		{
			FVector4 Color = { 0.0f, 0.0f, 0.0f, 1.0f };

			ss >> token >> tempVec.X >> tempVec.Y >> tempVec.Z;
			newObjRawData.Position.Emplace(tempVec);

			if (ss >> token >> Color.X >> Color.Y >> Color.Z >> Color.W)
			{
				newObjRawData.Color.Emplace(Color);
			}
			else
			{
				newObjRawData.Color.Emplace(Color);
			}
		}

		// normal
		if (line.substr(0, 3) == "vn ")
		{
			FVector norm;
			ss >> token >> norm.X >> norm.Y >> norm.Z;
			newObjRawData.Normal.Emplace(norm);
		}
		// texture
		else if (line.substr(0, 3) == "vt ")
		{
			FVector2 uv;
			ss >> token >> uv.X >> uv.Y;
			uv = UVToBasis(uv);
			newObjRawData.Tex.Emplace(uv);
		}
		// material lib
		else if (line.substr(0, 7) == "mtllib ")
		{
			FString mtlFilename;
			ss >> token >> mtlFilename;
			// mtl 파일명 추출
			FString path = filePath;
			size_t pathPos = path.find("/");
			mtlFilename = path.substr(0, pathPos);
			UE_LOG("%s", mtlFilename.c_str());
		}
		// materials
		else if (line.substr(0, 7) == "usemtl ")
		{
			// "usemtl " 이후 모든 문자열 추출
			FString mtlName = line.substr(7);
			// TODO : material 
		}
		// face
		else if (line.substr(0, 2) == "f ")
		{
			FIndex newFace;

			std::stringstream faceSS(line);
			FString faceToken;

			faceSS >> faceToken;

			while (faceSS >> faceToken)
			{
				// face의 정점 집합( v1/vt1/vn1 v2/vt2/vn2 ... )
				TArray<FString> faceSet;
				std::stringstream tokenSS(faceToken);
				// face 정점 집합의 원소 ( v/vt/vn )
				FString element;
				while (std::getline(tokenSS, element, '/'))
				{
					faceSet.Emplace(element);
				}

				// f v
				if (faceSet.size() == 1)
				{
					newFace.VertexIndices.Emplace(std::stoi(faceSet[0]) - 1);
				}
				// f v/vt
				else if (faceSet.size() == 2)
				{
					newFace.VertexIndices.Emplace(std::stoi(faceSet[0]) - 1);
					newFace.UVIndices.Emplace(std::stoi(faceSet[1]) - 1);
				}
				else if (faceSet.size() == 3)
				{
					// f v v//vn
					if (faceSet[1] == "")
					{
						newFace.VertexIndices.Emplace(std::stoi(faceSet[0]) - 1);
						newFace.NormalIndices.Emplace(std::stoi(faceSet[2]) - 1);
					}
					// f v/vt/vn
					else
					{
						newFace.VertexIndices.Emplace(std::stoi(faceSet[0]) - 1);
						newFace.UVIndices.Emplace(std::stoi(faceSet[1]) - 1);
						newFace.NormalIndices.Emplace(std::stoi(faceSet[2]) - 1);
					}
				}
				newStaticMesh->Indices.VertexIndices = TArray<uint32>(newFace.VertexIndices.begin(), newFace.VertexIndices.end());
				newStaticMesh->Indices.UVIndices = TArray<uint32>(newFace.UVIndices.begin(), newFace.UVIndices.end());
				newStaticMesh->Indices.NormalIndices = TArray<uint32>(newFace.NormalIndices.begin(), newFace.NormalIndices.end());

				// TODO : FaceMap
			}
		}
	}	

	return newStaticMesh;
}

