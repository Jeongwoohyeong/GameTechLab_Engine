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
	// s : "v", "v/vt", "v//vn", "v/vt/vn"
	v = vt = vn = 0;

	// slash 위치 저장용 변수
	int slash1 = -1, slash2 = -1;

	for (size_t i = 0;i < (size_t)s.size();i++)
	{
		// 문자열의 slash 위치 탐색
		if (s[i] == '/')
		{
			// slash는 문자열의 1번 째 index부터 시작
			// 먼저 첫 번째 슬래쉬를 검사해서 위치를 저장한다.
			if (slash1 < 0)
			{
				slash1 = i;
			}
			// slash1이 음수가 아닌 경우는 첫 번째 슬래쉬가 탐색된 경우
			// 슬래쉬가 탐색되었는데 첫 번째 슬래쉬가 존재하는 경우는 현재 슬래쉬가 두 번째 슬래쉬
			else
			{
				slash2 = i;
				break;
			}
		}
	}

	// 문자열 존재 여부를 검사하고 정수로 변환하는 람다함수
	auto toInt = [](const FString& a)->int
		{
			return a.empty() ? 0 : std::stoi(a);
		};

	// slash1이 음수 -> f v v v
	if (slash1 < 0)
	{
		v = toInt(s);
		return true;
	}
	// slash2가 음수 -> f v/vt v/vt v/vt
	if (slash2 < 0)
	{
		v = toInt(s.substr(0,slash1));
		vt = toInt(s.substr(slash1 + 1));
		return true;
	}

	// slash가 두개 존재하는 경우 : v/vt/vn, v//vn
	// 첫 위치에서 첫 번째 슬래쉬까지 파싱
	v = toInt(s.substr(0, slash1));
	// 첫 번째 슬래쉬와 두 번쨰 슬래쉬까지 파싱
	// 1/10/4 : slash1(1), slash2(4) 문자열의 2번 인덱스에서 2 글자 파싱
	FString mid = s.substr(slash1 + 1, slash2 - slash1 - 1);
	// 두 번째 슬래쉬 다음 위치부터 끝까지 파싱
	FString tail = s.substr(slash2 + 1);
	// 문자열이 존재하지 않으면 0
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
	// 최소 1개 섹션 시작
	outRawData.Sections.Emplace();
	FObjSection* currentSection = &outRawData.Sections.back();

	// 줄 단위로 파싱
	while (std::getline(file, line))
	{
		// 빈 줄, 주석 줄은 제외
		if (line.empty() || line[0] == '#')
		{
			continue;
		}

		// stringstream을 문자열 파싱에 사용
		// 공백, 개행을 제외함, 특정 문자로 tokenize 가능
		std::stringstream ss(line);
		FString token;
		ss >> token;

		if (token.empty())
		{
			continue;
		}

		// v : v v1 v2 v3
		if (token == "v")
		{
			FVector pos{};
			ss >> pos.X >> pos.Y >> pos.Z;
			outRawData.Position.Emplace(pos);
		}
		// vt : vt u v
		else if(token == "vt")
		{
			FVector2 uv{};
			ss >> uv.X >> uv.Y;
			uv = UVToBasis(uv);
			outRawData.Tex.Emplace(uv);
		}
		// vn : vn vn1 vn2 vn3
		else if (token == "vn")
		{
			FVector norm{};
			ss >> norm.X >> norm.Y >> norm.Z;
			outRawData.Normal.Emplace(norm);
		}
		// f : f "v/vt/vn"m "v/vt", "v//vn", "v"
		// face 파싱 함수로 파싱
		else if (token == "f")
		{
			FObjFace face{};
			FString faceToken;
			// ss에 남아 있는 f를 제외한 문자열 tokenize
			while (ss >> faceToken)
			{
				int32 v = 0, vt = 0, vn = 0;
				ParseFaceTriplet(faceToken, v, vt, vn);
				FObjVertexRef ref{};
				// 음수 인덱스 보정
				ref.V = ResolveIndex(v, (int32)outRawData.Position.size());
				ref.VT = (vt != 0) ? ResolveIndex(vt, (int32)outRawData.Tex.size()) : -1;
				ref.VN = (vn != 0) ? ResolveIndex(vn, (int32)outRawData.Normal.size()) : -1;

				// 정점 인덱스가 음수인 경우 이미 존재하는 인덱스 참조하면 된다.
				// 정점 개수가 n이고 정점이 -2면 n-2 인덱스
				// 배열의 크기보다 인덱스 값이 클 수 없음
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
			// 면의 경계가 3개 이상 -> 면을 형성함(최소 삼각형)
			if (face.Conners.size() >= 3)
			{
				// currentSection 변수는 포인터로 rawdata의 Sections변수 참조 중
				currentSection->Faces.Emplace(face);
			}			
		}
		// object : submesh로 이해하자.
		// group : 공통 속성이나 논리적 관계로 구성
		else if (token == "o" || token == "g")
		{
			FString name;
			std::getline(ss, name);
			while (!name.empty() && std::isspace((unsigned char)name.front()))
			{
				name.erase(name.begin());
			}
			// 그룹이나 오브젝트의 경우 새로운 섹션 추가
			outRawData.Sections.Emplace();
			currentSection = &outRawData.Sections.back();
			currentSection->Name = name;
		}
		else if (token == "usemtl")
		{
			// 현재 섹션의 mtl명 저장
			ss >> currentSection->MaterialName;
		}
		else if (token == "mtllib")
		{
			// .mtl 파일명 저장
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
	
	// .obj의 raw data의 section 순회
	for (const FObjSection& section : raw.Sections)
	{
		// 면이 아니면 건너뛰기
		if (section.Faces.empty())
		{
			continue;
		}

		FMeshSection outSection{};
		outSection.Name = section.Name;
		outSection.MaterialName = section.MaterialName;
		// 섹션의 시작 인덱스 기록
		outSection.IndexStart = outMesh.Indices.Num();

		// 중복정점 제거 용 캐시맵
		TMap<FVertexKey, uint32, FVertexKeyHash> cache;

		// FVertexKey(v, vt, vn)의 정점 생성
		// 또는 존재하는 정점 인덱스 반환
		auto emit = [&](const FVertexKey& key) -> uint32
			{				
				auto it = cache.find(key);
				// 캐시에 존재하는 정점 인덱스 반환
				if (it != cache.end())
				{
					return it->second;
				}

				FNormalVertex vertex{};
				// raw data에서 vertex position 추출
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
				// 기본 색상
				vertex.Color = FVector4(0.4f, 0.4f, 0.4f, 1.0f);

				// 정점 배열의 현재 개수가 새로운 인덱스
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

				// 람다함수 emit은 내부에서 vertices배열의 개수를 인덱스로 반환한다.
				// 함수 내부에서 vertices배열에 vertex를 저장한다.
				// 저장하고 반환하면 저장된 정점의 다음 인덱스가 되어 저장 전에 반환
				uint32 ia = emit(ka);
				uint32 ib = emit(kb);
				uint32 ic = emit(kc);

				// emit함수가 반환한 인덱스를 indices배열에 저장
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

		// 현재 섹션의 인덱스 개수 계산
		// indices배열의 원소 개수에서 시작 인덱스를 제외하면 섹션의 인덱스 개수
		outSection.IndexCount = outMesh.Indices.Num() - outSection.IndexStart;
		// 
		if (outSection.IndexCount > 0)
		{
			outMesh.Sections.Emplace(outSection);
		}
	}
	outMesh.IndexNum = outMesh.Indices.Num();

	outMesh.IndexNum = outMesh.Indices.Num();
	// 필요하면 노말 재계산 
	/*if (opt.bIsRecalculateNormals)
	{

	}*/

	return true;
}
