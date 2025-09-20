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
	FObjInfo raw{};

	if (!ParseObjRaw(filePath, raw))
	{
		UE_LOG("obj 파싱 실패");
		return nullptr;
	}

	FStaticMesh* newStaticMesh = new FStaticMesh();
	newStaticMesh->PathFileName = filePath;

	FObjImportOption opt{};

	if (!CookObjToStaticMesh(raw, opt, *newStaticMesh))
	{
		UE_LOG("obj cook 실패");
		delete newStaticMesh;
		return nullptr;
	}

	{
		uint32 pos = filePath.find('/');
		FString name = filePath.substr(pos + 1);
		UE_LOG("FObjParser: %s 로드 완료", name.c_str());
	}

	return newStaticMesh;
}

bool FObjParser::ParseFaceTriplet(const FString& s, int32& v, int32& vt, int32& vn)
{
	v = vt = vn = 0;
	int slash1 = -1, slash2 = -1;

	for (size_t i = 0;i < (size_t)s.size();i++)
	{
		if (s[i] == '/')
		{
			if (slash1 < 0)
			{
				slash1 = i;
			}
			else
			{
				slash2 = i;
				break;
			}
		}
	}

	auto toInt = [](const FString& a)->int
		{
			return a.empty() ? 0 : std::stoi(a);
		};

	if (slash1 < 0)
	{
		v = toInt(s);
		return true;
	}
	if (slash2 < 0)
	{
		v = toInt(s.substr(0,slash1));
		vt = toInt(s.substr(slash1 + 1));
		return true;
	}
	v = toInt(s.substr(0, slash1));
	FString mid = s.substr(slash1 + 1, slash2 - slash1 - 1);
	FString tail = s.substr(slash2 + 1);
	vt = mid.empty() ? 0 : toInt(mid);
	vn = tail.empty() ? 0 : toInt(tail);

	return true;
}

bool FObjParser::ParseObjRaw(const FString& filePath, FObjInfo& outRawData)
{
	ifstream file(filePath);

	if (!file.is_open())
	{
		UE_LOG("파일 열기 실패");
		return false;
	}

	outRawData = FObjInfo();

	FString line;
	outRawData.Sections.Emplace();
	FObjSection* currentSection = &outRawData.Sections.back();

	while (std::getline(file, line))
	{
		if (line.empty() || line[0] == '#')
		{
			continue;
		}

		std::stringstream ss(line);
		FString token;
		ss >> token;

		if (token.empty())
		{
			continue;
		}

		if (token == "v")
		{
			FVector pos{};
			ss >> pos.X >> pos.Y >> pos.Z;
			outRawData.Position.Emplace(pos);
		}
		else if(token == "vt")
		{
			FVector2 uv{};
			ss >> uv.X >> uv.Y;
			uv = UVToBasis(uv);
			outRawData.Tex.Emplace(uv);
		}
		else if (token == "vn")
		{
			FVector norm{};
			ss >> norm.X >> norm.Y >> norm.Z;
			outRawData.Normal.Emplace(norm);
		}
		else if (token == "f")
		{
			FObjFace face{};
			FString faceToken;
			while (ss >> faceToken)
			{
				int32 v = 0, vt = 0, vn = 0;
				ParseFaceTriplet(faceToken, v, vt, vn);
				FObjVertexRef ref{};
				ref.V = ResolveIndex(v, (int32)outRawData.Position.size());
				ref.VT = (vt != 0) ? ResolveIndex(vt, (int32)outRawData.Tex.size()) : -1;
				ref.VN = (vn != 0) ? ResolveIndex(vn, (int32)outRawData.Normal.size()) : -1;

				if (ref.V < 0 || ref.V >= (int32)outRawData.Position.size())
				{
					continue;
				}
				if (ref.VT >= (int32)outRawData.Tex.size())
				{
					ref.VT = -1;
				}
				if (ref.VN >= (int32)outRawData.Normal.size())
				{
					ref.VN = -1;
				}
				face.Conners.Emplace(ref);
			}
			if (face.Conners.size() >= 3)
			{
				currentSection->Faces.Emplace(face);
			}			
		}
		else if (token == "o" || token == "g")
		{
			FString name;
			std::getline(ss, name);
			while (!name.empty() && std::isspace((unsigned char)name.front()))
			{
				name.erase(name.begin());
			}
			outRawData.Sections.Emplace();
			currentSection = &outRawData.Sections.back();
			currentSection->Name = name;
		}
		else if (token == "usemtl")
		{
			ss >> currentSection->MaterialName;
		}
		else if (token == "mtllin")
		{
			ss >> outRawData.Mtllib;
		}
		else
		{
			// 기타
		}
	}

	return true;
}

bool FObjParser::CookObjToStaticMesh(const FObjInfo& raw, const FObjImportOption& opt, FStaticMesh& outMesh)
{
	outMesh.Vertices.clear();
	outMesh.Indices.clear();
	outMesh.Sections.clear();
	outMesh.Mtllib = raw.Mtllib;
	

	for (const FObjSection& section : raw.Sections)
	{
		if (section.Faces.empty())
		{
			continue;
		}

		FMeshSection outSection{};
		outSection.Name = section.Name;
		outSection.MaterialName = section.MaterialName;
		outSection.IndexStart = outMesh.Indices.Num();

		TMap<FVertexKey, uint32, FVertexKeyHash> cache;

		auto emit = [&](const FVertexKey& key) -> uint32
			{
				auto it = cache.find(key);
				if (it != cache.end())
				{
					return it->second;
				}

				FNormalVertex vertex{};
				vertex.Position = raw.Position[key.v];

				if (key.vt >= 0)
				{
					vertex.Tex = raw.Tex[key.vt];
					if (opt.bIsInvertTexV)
					{
						vertex.Tex = UVToBasis(vertex.Tex);
					}
				}
				else
				{
					vertex.Tex = FVector2(0, 0);
				}

				if (key.vn >= 0)
				{
					vertex.Normal = raw.Normal[key.vn];
				}
				else
				{
					// 임시 노말
					vertex.Normal = FVector(0, 0, 1);
				}
				vertex.Color = FVector4(0.4f, 0.4f, 0.4f, 1.0f);

				uint32 newIdx = (uint32)outMesh.Vertices.Num();
				outMesh.Vertices.Emplace(vertex);
				cache.Emplace(key, newIdx);
				return newIdx;
			};

		for (const FObjFace& face : section.Faces)
		{
			if (face.Conners.size() < 3)
			{
				continue;
			}
			const FObjVertexRef& a = face.Conners[0];

			for (size_t i = 1; i + 1 < face.Conners.size(); i++)
			{
				const FObjVertexRef& b = face.Conners[i];
				const FObjVertexRef& c = face.Conners[i + 1];

				FVertexKey ka{ a.V, a.VT, a.VN };
				FVertexKey kb{ b.V, b.VT, b.VN };
				FVertexKey kc{ c.V, c.VT, c.VN };

				uint32 ia = emit(ka);
				uint32 ib = emit(kb);
				uint32 ic = emit(kc);

				if (!opt.bIsFlipWinding)
				{
					outMesh.Indices.Emplace(ia);
					outMesh.Indices.Emplace(ib);
					outMesh.Indices.Emplace(ic);
				}
				else
				{
					outMesh.Indices.Emplace(ia);
					outMesh.Indices.Emplace(ic);
					outMesh.Indices.Emplace(ib);
				}
			}
		}

		outMesh.IndexNum = outMesh.Indices.Num() - outSection.IndexStart;
		if (outSection.IndexCount > 0)
		{
			outMesh.Sections.Emplace(outSection);
		}
	}

	// 필요하면 노말 재계산 
	/*if (opt.bIsRecalculateNormals)
	{

	}*/

	return true;
}
