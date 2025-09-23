#include "pch.h"
#include "ObjManager.h"
#include "Manager/Path/PathManager.h"
#include "public/Mesh/StaticMesh.h"
#include "Public/Core/ObjectIterator.h"
#include "Public/Utility/MtlParser.h"
#include "Mesh/Material.h"

TMap<FString, FStaticMesh*> FObjManager::ObjStaticMap{};

TMap<FString, UMaterial*> FObjManager::Materials{};

FMtlParser* FObjManager::MtlManager{};

IMPLEMENT_SINGLETON(FObjManager)

FObjManager::FObjManager()
{
	MtlManager = new FMtlParser(&Materials);
}

FObjManager::~FObjManager()
{
	if (!Materials.empty())
	{
		ReleaseMtlInfo();
	}

	if (!ObjStaticMap.empty())
	{
		ReleaseStaticMesh();
	}

	if (MtlManager)
	{
		delete MtlManager;
	}
}

void FObjManager::Intialize()
{
	LoadObjStaticMeshAsset("Data/cube-tex.obj");
	LoadObjStaticMeshAsset("Data/sphere.obj");
	LoadObjStaticMeshAsset("Data/triangle.obj");
	LoadObjStaticMeshAsset("Data/square.obj");
	LoadObjStaticMeshAsset("Data/minion.obj");
	//LoadObjStaticMeshAsset("Data/trees9.obj");
	
}

UStaticMesh* FObjManager::LoadObjStaticMesh(const FString& PathFileName)
{
	UStaticMesh* Result = nullptr;
	for (TObjectIterator<UStaticMesh> It; It; ++It)
	{
		UStaticMesh* StaticMesh = *It;
		if (StaticMesh && (StaticMesh->GetAssetPathFileName() == PathFileName))
		{
			Result = *It;
			break;
		}
	}

	if (Result == nullptr)
	{
		FStaticMesh* Asset = FObjManager::LoadObjStaticMeshAsset(PathFileName);
		UStaticMesh* StaticMesh = NewObject<UStaticMesh>();
		StaticMesh->SetStaticMeshAsset(Asset);
		Result = StaticMesh;
	}
	
	if (PathFileName.find("sphere") != PathFileName.npos)
	{
		Result->SetPrimtiveType(EPrimitiveType::Sphere);
	}
	else if (PathFileName.find("cube") != PathFileName.npos)
	{
		Result->SetPrimtiveType(EPrimitiveType::Cube);
	}
	else if (PathFileName.find("square") != PathFileName.npos)
	{
		Result->SetPrimtiveType(EPrimitiveType::Square);
	}
	else if (PathFileName.find("triangle") != PathFileName.npos)
	{
		Result->SetPrimtiveType(EPrimitiveType::Triangle);
	}

	return Result;
}

FStaticMesh* FObjManager::LoadObjStaticMeshAsset(const FString& pathFileName)
{	
	auto It = ObjStaticMap.find(pathFileName);	
	if (It != ObjStaticMap.end())
	{
		return (*It).second;
	}
	FStaticMesh* NewStaticMesh = FObjManager::LoadObj(pathFileName);	
	ObjStaticMap.emplace(pathFileName, NewStaticMesh);

	if (!MtlManager->ParseMtl(NewStaticMesh->Mtllib))
	{
		UE_LOG("LoadObjStaticMeshAsset : Mtl Parsing 실패");
	}

	// 디버깅용 로그
	{
		/*if (!NewStaticMesh->PathFileName.empty())
		{
			UE_LOG("PathFileName %s", NewStaticMesh->PathFileName.c_str());
			UE_LOG("Mtllib %s", NewStaticMesh->Mtllib.c_str());
			if (!NewStaticMesh->Sections.empty())
			{
				for (const auto& Section : NewStaticMesh->Sections)
				{
					if (!Section.Name.empty())
					{
						UE_LOG("Section name %s", Section.Name.c_str());
					}

					if (!Section.MaterialName.empty())
					{
						UE_LOG("Material name %s", Section.MaterialName.c_str());
					}
				}
			}
		}*/
	}

	return NewStaticMesh;
}

FStaticMesh* FObjManager::LoadObj(const FString& filePath)
{	
	FObjInfo Raw{};

	if (!ParseObjRaw(filePath, Raw))
	{
		UE_LOG("obj 파싱 실패");
		return nullptr;
	}

	FStaticMesh* NewStaticMesh = new FStaticMesh();
	NewStaticMesh->PathFileName = filePath;

	FObjImportOption Opt{};

	if (!CookObjToStaticMesh(Raw, Opt, *NewStaticMesh))
	{
		UE_LOG("obj cook 실패");
		delete NewStaticMesh;
		return nullptr;
	}

	{
		uint32 Pos = filePath.find_last_of("/\\");
		FString Name = filePath.substr(Pos + 1);
		UE_LOG("FObjManager: %s 로드 완료", Name.c_str());
	}

	return NewStaticMesh;
}

bool FObjManager::ParseFaceTriplet(const FString& S, int32& V, int32& Vt, int32& Vn)
{
	// s : "v", "v/vt", "v//vn", "v/vt/vn"
	V = Vt = Vn = 0;

	// slash 위치 저장용 변수
	int Slash1 = -1, Slash2 = -1;

	for (size_t i = 0;i < (size_t)S.size();i++)
	{
		// 문자열의 slash 위치 탐색
		if (S[i] == '/')
		{
			// slash는 문자열의 1번 째 index부터 시작
			// 먼저 첫 번째 슬래쉬를 검사해서 위치를 저장한다.
			if (Slash1 < 0)
			{
				Slash1 = i;
			}
			// slash1이 음수가 아닌 경우는 첫 번째 슬래쉬가 탐색된 경우
			// 슬래쉬가 탐색되었는데 첫 번째 슬래쉬가 존재하는 경우는 현재 슬래쉬가 두 번째 슬래쉬
			else
			{
				Slash2 = i;
				break;
			}
		}
	}

	// 문자열 존재 여부를 검사하고 정수로 변환하는 람다함수
	auto ToInt = [](const FString& a)->int
		{
			return a.empty() ? 0 : std::stoi(a);
		};

	// slash1이 음수 -> f v v v
	if (Slash1 < 0)
	{
		V = ToInt(S);
		return true;
	}
	// slash2가 음수 -> f v/vt v/vt v/vt
	if (Slash2 < 0)
	{
		V = ToInt(S.substr(0,Slash1));
		Vt = ToInt(S.substr(Slash1 + 1));
		return true;
	}

	// slash가 두개 존재하는 경우 : v/vt/vn, v//vn
	// 첫 위치에서 첫 번째 슬래쉬까지 파싱
	V = ToInt(S.substr(0, Slash1));
	// 첫 번째 슬래쉬와 두 번쨰 슬래쉬까지 파싱
	// 1/10/4 : slash1(1), slash2(4) 문자열의 2번 인덱스에서 2 글자 파싱
	FString Mid = S.substr(Slash1 + 1, Slash2 - Slash1 - 1);
	// 두 번째 슬래쉬 다음 위치부터 끝까지 파싱
	FString Tail = S.substr(Slash2 + 1);
	// 문자열이 존재하지 않으면 0
	Vt = Mid.empty() ? 0 : ToInt(Mid);
	Vn = Tail.empty() ? 0 : ToInt(Tail);

	return true;
}

bool FObjManager::ParseObjRaw(const FString& FilePath, FObjInfo& OutRawData)
{
	ifstream File(FilePath);

	if (!File.is_open())
	{
		UE_LOG("Obj 파일 열기 실패");
		return false;
	}

	OutRawData = FObjInfo();

	uint32 Pos = FilePath.find_last_of("/\\");
	FString Path = FilePath.substr(0, Pos + 1);	

	FString Line;
	// 최소 1개 섹션 시작
	OutRawData.Sections.Emplace();
	FObjSection* CurrentSection = &OutRawData.Sections.back();

	// 줄 단위로 파싱
	while (std::getline(File, Line))
	{
		// 빈 줄, 주석 줄은 제외
		if (Line.empty() || Line[0] == '#')
		{
			continue;
		}

		// stringstream을 문자열 파싱에 사용
		// 공백, 개행을 제외함, 특정 문자로 tokenize 가능
		std::stringstream Ss(Line);
		FString Token;
		Ss >> Token;

		if (Token.empty())
		{
			continue;
		}

		// v : v v1 v2 v3
		if (Token == "v")
		{
			FVector Pos{};
			Ss >> Pos.X >> Pos.Y >> Pos.Z;
			OutRawData.Position.Emplace(Pos);
		}
		// vt : vt u v
		else if(Token == "vt")
		{
			FVector2 UV{};
			Ss >> UV.X >> UV.Y;
			UV = UVToBasis(UV);
			OutRawData.Tex.Emplace(UV);
		}
		// vn : vn vn1 vn2 vn3
		else if (Token == "vn")
		{
			FVector Norm{};
			Ss >> Norm.X >> Norm.Y >> Norm.Z;
			OutRawData.Normal.Emplace(Norm);
		}
		// f : f "v/vt/vn"m "v/vt", "v//vn", "v"
		// face 파싱 함수로 파싱
		else if (Token == "f")
		{
			FObjFace Face{};
			FString FaceToken;
			// ss에 남아 있는 f를 제외한 문자열 tokenize
			while (Ss >> FaceToken)
			{
				int32 V = 0, Vt = 0, Vn = 0;
				ParseFaceTriplet(FaceToken, V, Vt, Vn);
				FObjVertexRef Ref{};
				// 음수 인덱스 보정
				Ref.V = ResolveIndex(V, (int32)OutRawData.Position.size());
				Ref.VT = (Vt != 0) ? ResolveIndex(Vt, (int32)OutRawData.Tex.size()) : -1;
				Ref.VN = (Vn != 0) ? ResolveIndex(Vn, (int32)OutRawData.Normal.size()) : -1;

				// 정점 인덱스가 음수인 경우 이미 존재하는 인덱스 참조하면 된다.
				// 정점 개수가 n이고 정점이 -2면 n-2 인덱스
				// 배열의 크기보다 인덱스 값이 클 수 없음
				if (Ref.V < 0 || Ref.V >= (int32)OutRawData.Position.size())
				{
					continue;
				}
				if (Ref.VT >= (int32)OutRawData.Tex.size())
				{
					Ref.VT = -1;
				}
				if (Ref.VN >= (int32)OutRawData.Normal.size())
				{
					Ref.VN = -1;
				}
				Face.Conners.Emplace(Ref);
			}
			// 면의 경계가 3개 이상 -> 면을 형성함(최소 삼각형)
			if (Face.Conners.size() >= 3)
			{
				// currentSection 변수는 포인터로 rawdata의 Sections변수 참조 중				
				CurrentSection->Faces.Emplace(Face);
			}
		}
		// object : submesh로 이해하자.
		// group : 공통 속성이나 논리적 관계로 구성
		else if (Token == "o" || Token == "g")
		{
			FString Name;
			std::getline(Ss, Name);
			while (!Name.empty() && std::isspace((unsigned char)Name.front()))
			{
				Name.erase(Name.begin());
			}
			// 그룹이나 오브젝트의 경우 새로운 섹션 추가
			OutRawData.Sections.Emplace();
			CurrentSection = &OutRawData.Sections.back();
			CurrentSection->Name = Name;
		}
		else if (Token == "usemtl")
		{
			// cli 수정코드
			if (!CurrentSection->Faces.empty())
			{
				OutRawData.Sections.Emplace();
				CurrentSection = &OutRawData.Sections.back();
			}
			// usemtl이 시작되면 새로운 섹션 추가
			// 원래 코드
			/*OutRawData.Sections.Emplace();
			CurrentSection = &OutRawData.Sections.back();*/
			// 현재 섹션의 mtl명 저장
			Ss >> CurrentSection->MaterialName;
			//UE_LOG("mtl name %s", CurrentSection->MaterialName.c_str());

		}
		else if (Token == "mtllib")
		{
			// .mtl 파일명 저장
			FString MtlName;
			Ss >> MtlName;
			OutRawData.Mtllib = Path.append(MtlName);
			//UE_LOG("Mtllib %s", OutRawData.Mtllib.c_str());
		}
		else
		{
			// 기타
		}
	}

	return true;
}

bool FObjManager::CookObjToStaticMesh(const FObjInfo& Raw, const FObjImportOption& Opt, FStaticMesh& OutMesh)
{
	OutMesh.Vertices.clear();
	OutMesh.Indices.clear();
	OutMesh.Sections.clear();
	OutMesh.Mtllib = Raw.Mtllib;	
	
	// .obj의 raw data의 section 순회
	for (const FObjSection& Section : Raw.Sections)
	{
		// 면이 아니면 건너뛰기
		if (Section.Faces.empty())
		{
			continue;
		}

		FMeshSection OutSection{};
		OutSection.Name = Section.Name;
		OutSection.MaterialName = Section.MaterialName;
		// 섹션의 시작 인덱스 기록
		OutSection.IndexStart = OutMesh.Indices.Num();

		// 중복정점 제거 용 캐시맵
		TMap<FVertexKey, uint32, FVertexKeyHash> Cache;

		// FVertexKey(v, vt, vn)의 정점 생성
		// 또는 존재하는 정점 인덱스 반환
		auto Emit = [&](const FVertexKey& key) -> uint32
			{				
				auto It = Cache.find(key);
				// 캐시에 존재하는 정점 인덱스 반환
				if (It != Cache.end())
				{
					return It->second;
				}

				FNormalVertex Vertex{};
				// raw data에서 vertex position 추출
				Vertex.Position = Raw.Position[key.v];

				if (key.vt >= 0)
				{
					Vertex.Tex = Raw.Tex[key.vt];
					if (Opt.bIsInvertTexV)
					{
						Vertex.Tex = UVToBasis(Vertex.Tex);
					}
				}
				else
				{
					Vertex.Tex = FVector2(0, 0);
				}

				if (key.vn >= 0)
				{
					Vertex.Normal = Raw.Normal[key.vn];
				}
				else
				{
					// 임시 노말
					Vertex.Normal = FVector(0, 0, 1);
				}
				// 기본 색상
				Vertex.Color = FVector4(0.4f, 0.4f, 0.4f, 1.0f);

				// 정점 배열의 현재 개수가 새로운 인덱스
				uint32 NewIdx = (uint32)OutMesh.Vertices.Num();
				OutMesh.Vertices.Emplace(Vertex);
				Cache.Emplace(key, NewIdx);
				return NewIdx;
			};

		for (const FObjFace& Face : Section.Faces)
		{
			if (Face.Conners.size() < 3)
			{
				continue;
			}
			const FObjVertexRef& A = Face.Conners[0];

			for (size_t i = 1; i + 1 < Face.Conners.size(); i++)
			{
				const FObjVertexRef& B = Face.Conners[i];
				const FObjVertexRef& C = Face.Conners[i + 1];

				FVertexKey Ka{ A.V, A.VT, A.VN };
				FVertexKey Kb{ B.V, B.VT, B.VN };
				FVertexKey Kc{ C.V, C.VT, C.VN };

				// 람다함수 emit은 내부에서 vertices배열의 개수를 인덱스로 반환한다.
				// 함수 내부에서 vertices배열에 vertex를 저장한다.
				// 저장하고 반환하면 저장된 정점의 다음 인덱스가 되어 저장 전에 반환
				uint32 Ia = Emit(Ka);
				uint32 Ib = Emit(Kb);
				uint32 Ic = Emit(Kc);

				// emit함수가 반환한 인덱스를 indices배열에 저장
				if (!Opt.bIsFlipWinding)
				{
					OutMesh.Indices.Emplace(Ia);
					OutMesh.Indices.Emplace(Ib);
					OutMesh.Indices.Emplace(Ic);
				}
				else
				{
					OutMesh.Indices.Emplace(Ia);
					OutMesh.Indices.Emplace(Ic);
					OutMesh.Indices.Emplace(Ib);
				}
			}			
		}

		// 현재 섹션의 인덱스 개수 계산
		// indices배열의 원소 개수에서 시작 인덱스를 제외하면 섹션의 인덱스 개수
		OutSection.IndexCount = OutMesh.Indices.Num() - OutSection.IndexStart;
		// 
		if (OutSection.IndexCount > 0)
		{
			OutMesh.Sections.Emplace(OutSection);
		}
	}
	OutMesh.IndexNum = OutMesh.Indices.Num();
	for (auto& Section : OutMesh.Sections)
	{
		UE_LOG("mtl name %s", Section.MaterialName.c_str());
		UE_LOG("Start Index %d", Section.IndexStart);
		UE_LOG("Index Count %d", Section.IndexCount);
	}

	/*if (OutMesh.Mtllib == "Data/minion.mtl")
	{
		for (const auto& e : OutMesh.Sections)
		{
			UE_LOG("mtl name %s", e.MaterialName.c_str());
			UE_LOG("group name %s", e.Name.c_str());
		}
	}*/

	// 필요하면 노말 재계산 
	/*if (opt.bIsRecalculateNormals)
	{

	}*/

	return true;
}

//bool FObjManager::LoadMtlMap(const FString& MtlFileName)
//{
//	//UE_LOG("mtlfilename %s", MtlFileName.c_str());
//
//	// find의 결과가 end가 아니면 존재
//	bool bIsExist = (MtlFileMap.find(MtlFileName) != MtlFileMap.end());
//
//	// 존재하지 않으면 mtlfile parsing
//	if (!bIsExist)
//	{
//		// 파싱 성공 여부 저장
//		// 성공 : true
//		// 실패 : false
//		bIsExist = MtlManager->ParseMtl(MtlFileName);		
//	}
//
//	if (!bIsExist)
//	{
//		UE_LOG("%s Parsing Fail", MtlFileName.c_str());
//	}
//
//	return bIsExist;
//}

void FObjManager::ReleaseStaticMesh()
{
	for (auto It = ObjStaticMap.begin(); It != ObjStaticMap.end(); It++)
	{
		if ((*It).second)
		{
			delete (*It).second;
			(*It).second = nullptr;
		}
	}
}

void FObjManager::ReleaseMtlInfo()
{
	for (auto& Pair : Materials)
	{
		delete Pair.second;
	}
	Materials.Empty();
}
