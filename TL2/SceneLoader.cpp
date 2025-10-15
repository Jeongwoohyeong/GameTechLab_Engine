#include "pch.h"
#include "SceneLoader.h"

#include <algorithm>
#include <iomanip>

static bool ParsePerspectiveCamera(const JSON& Root, FPerspectiveCameraData& OutCam)
{
    if (!Root.hasKey("PerspectiveCamera"))
        return false;

    const JSON& Cam = Root.at("PerspectiveCamera");

    // 배열형 벡터(Location, Rotation) 파싱 (스칼라 실패 시 무시)
    auto readVec3 = [](JSON arr, FVector& outVec)
        {
            try
            {
                outVec = FVector(
                    (float)arr[0].ToFloat(),
                    (float)arr[1].ToFloat(),
                    (float)arr[2].ToFloat());
            }
            catch (...) {} // 실패 시 기본값 유지
        };

    if (Cam.hasKey("Location"))
        readVec3(Cam.at("Location"), OutCam.Location);
    if (Cam.hasKey("Rotation"))
        readVec3(Cam.at("Rotation"), OutCam.Rotation);

    // 스칼라 또는 [스칼라] 모두 허용
    auto readScalarFlexible = [](const JSON& parent, const char* key, float& outVal)
        {
            if (!parent.hasKey(key)) return;
            const JSON& node = parent.at(key);
            try
            {
                // 배열 형태 시도 (예: "FOV": [60.0])
                outVal = (float)node.at(0).ToFloat();
            }
            catch (...)
            {
                // 스칼라 (예: "FOV": 60.0)
                outVal = (float)node.ToFloat();
            }
        };

    readScalarFlexible(Cam, "FOV", OutCam.FOV);
    readScalarFlexible(Cam, "NearClip", OutCam.NearClip);
    readScalarFlexible(Cam, "FarClip", OutCam.FarClip);

    return true;
}

// ========================================
// Version 2 API Implementation
// ========================================

void FSceneLoader::Save(const FSceneData& SceneData, const FString& SceneName)
{
    namespace fs = std::filesystem;
    fs::path outPath(SceneName);
    if (!outPath.has_parent_path())
        outPath = fs::path("Scene") / outPath;
    if (outPath.extension().string() != ".Scene")
        outPath.replace_extension(".Scene");
    std::error_code ec;
    fs::create_directories(outPath.parent_path(), ec);

    auto NormalizePath = [](FString Path) -> FString
    {
        // 절대 경로를 프로젝트 기준 상대 경로로 변환
        fs::path absPath = fs::absolute(Path);
        fs::path currentPath = fs::current_path();

        std::error_code ec;
        fs::path relativePath = fs::relative(absPath, currentPath, ec);

        // 상대 경로 변환 실패 시 원본 경로 사용
        FString result = ec ? Path : relativePath.string();

        // 백슬래시를 슬래시로 변환 (크로스 플랫폼 호환)
        for (auto& ch : result)
        {
            if (ch == '\\') ch = '/';
        }
        return result;
    };

    std::ostringstream oss;
    oss.setf(std::ios::fixed);
    oss << std::setprecision(6);
    oss << std::boolalpha;

    auto writeVec3 = [&](const char* name, const FVector& v, int indent)
    {
        std::string tabs(indent, ' ');
        oss << tabs << "\"" << name << "\" : [" << v.X << ", " << v.Y << ", " << v.Z << "]";
    };

    // Root
    oss << "{\n";
    oss << "  \"Version\" : " << SceneData.Version << ",\n";
    oss << "  \"NextUUID\" : " << SceneData.NextUUID << ",\n";

    // Camera
    oss << "  \"PerspectiveCamera\" : {\n";
    oss << "    \"FOV\" : [" << SceneData.Camera.FOV << "],\n";
    oss << "    \"FarClip\" : [" << SceneData.Camera.FarClip << "],\n";
    writeVec3("Location", SceneData.Camera.Location, 4); oss << ",\n";
    oss << "    \"NearClip\" : [" << SceneData.Camera.NearClip << "],\n";
    writeVec3("Rotation", SceneData.Camera.Rotation, 4); oss << "\n";
    oss << "  },\n";

    // Actors
    oss << "  \"Actors\" : [\n";
    for (size_t i = 0; i < SceneData.Actors.size(); ++i)
    {
        const FActorData& Actor = SceneData.Actors[i];
        oss << "    {\n";
        oss << "      \"UUID\" : " << Actor.UUID << ",\n";
        oss << "      \"Type\" : \"" << Actor.Type << "\",\n";
        oss << "      \"Name\" : \"" << Actor.Name << "\",\n";
        oss << "      \"RootComponentUUID\" : " << Actor.RootComponentUUID << "\n";
        oss << "    }" << (i + 1 < SceneData.Actors.size() ? "," : "") << "\n";
    }
    oss << "  ],\n";

    // Components
    oss << "  \"Components\" : [\n";
    for (size_t i = 0; i < SceneData.Components.size(); ++i)
    {
        FComponentData* Component = SceneData.Components[i];
        FSceneComponentData* SceneComponent = dynamic_cast<FSceneComponentData*>(SceneData.Components[i]);

        oss << "    {\n";
        oss << "      \"UUID\" : " << Component->UUID << ",\n";
        oss << "      \"OwnerActorUUID\" : " << Component->OwnerActorUUID << ",\n";
        oss << "      \"IsHierarchical\" : \"" << Component->IsHierarchical << "\",\n";
        oss << "      \"Type\" : \"" << Component->Type << "\",\n";

        // 계층 컴포넌트일때
        if (SceneComponent)
        {
            oss << "      \"ParentComponentUUID\" : " << SceneComponent->ParentComponentUUID << ",\n";
            writeVec3("RelativeLocation", SceneComponent->RelativeLocation, 6); oss << ",\n";
            writeVec3("RelativeRotation", SceneComponent->RelativeRotation, 6); oss << ",\n";
            writeVec3("RelativeScale", SceneComponent->RelativeScale, 6);

            FStaticMeshComponentData* StaticMeshData = dynamic_cast<FStaticMeshComponentData*>(SceneComponent);
            FDecalComponentData* DecalData = dynamic_cast<FDecalComponentData*>(SceneComponent);
            FBillboardComponentData* BillboardData = dynamic_cast<FBillboardComponentData*>(SceneComponent);
            FTextComponentData* TextData = dynamic_cast<FTextComponentData*>(SceneComponent);
            FHeightFogComponentData* HeightFogData = dynamic_cast<FHeightFogComponentData*>(SceneComponent);
            FFireBallComponentData* FireBallData = dynamic_cast<FFireBallComponentData*>(SceneComponent);

            if (StaticMeshData)
            {
                if (!StaticMeshData->StaticMesh.empty())
                {
                    oss << ",\n";
                    FString AssetPath = NormalizePath(StaticMeshData->StaticMesh);
                    oss << "      \"StaticMesh\" : \"" << AssetPath << "\"";

                    /*
                    * 추후 Material 저장 및 복원을 위한 포석
                    if (!Comp.Materials.empty())
                    {
                        oss << ",\n";
                        oss << "      \"Materials\" : [";
                        for (size_t m = 0; m < Comp.Materials.size(); ++m)
                        {
                            oss << "\"" << Comp.Materials[m] << "\"";
                            if (m + 1 < Comp.Materials.size()) oss << ", ";
                        }
                        oss << "]";
                    }*/
                }
            }
            else if (DecalData)
            {
                if (!DecalData->Texture.empty())
                {
                    oss << ",\n";
                    FString AssetPath = NormalizePath(DecalData->Texture);
                    oss << "      \"Texture\" : \"" << AssetPath << "\"";

                    oss << ",\n";
                    oss << "      \"FadeType\" : \"" << static_cast<int32>(DecalData->FadeType) << "\"";

                    oss << ",\n";
                    oss << "      \"Duration\" : \"" << DecalData->Duration << "\"";

                    oss << ",\n";
                    oss << "      \"Min\" : \"" << DecalData->Min << "\"";

                    oss << ",\n";
                    oss << "      \"Max\" : \"" << DecalData->Max << "\"";

                    oss << ",\n";
                    oss << "      \"Alpha\" : \"" << DecalData->Alpha << "\"";

                    oss << ",\n";
                    oss << "      \"FadeEnabled\" : \"" << DecalData->bIsFadeEnabled << "\"";

                    oss << ",\n";
                    oss << "      \"FadeStart\" : \"" << DecalData->bIsFadeStart << "\"";

                    oss << ",\n";
                    oss << "      \"Loop\" : \"" << DecalData->bIsLoop << "\"";

                    oss << ",\n";
                    oss << "      \"ElapsedTime\" : \"" << DecalData->ElapsedTime << "\"";

                }
            }
            else if (BillboardData)
            {
                if (!BillboardData->Texture.empty())
                {
                    oss << ",\n";
                    FString AssetPath = NormalizePath(BillboardData->Texture);
                    oss << "      \"Texture\" : \"" << AssetPath << "\"";
                }
            }
            else if (TextData)
            {
                if (!TextData->Text.empty())
                {
                    oss << ",\n";
                    FString AssetPath = NormalizePath(TextData->Text);
                    oss << "      \"Text\" : \"" << AssetPath << "\"";
                }
            }
            else if (HeightFogData)
            {
                oss << ",\n";
                oss << "      \"FogDensity\" : \"" << HeightFogData->FogDensity << "\"";

                oss << ",\n";
                oss << "      \"FogHeightFalloff\" : \"" << HeightFogData->FogHeightFalloff << "\"";

                oss << ",\n";
                oss << "      \"StartDistance\" : \"" << HeightFogData->StartDistance << "\"";

                oss << ",\n";
                oss << "      \"FogCutoffDistance\" : \"" << HeightFogData->FogCutoffDistance << "\"";

                oss << ",\n";
                oss << "      \"FogMaxOpacity\" : \"" << HeightFogData->FogMaxOpacity << "\"";

                oss << ",\n";
                oss << "      \"FogInscatteringColorR\" : \"" << HeightFogData->FogInscatteringColor.R << "\"";

                oss << ",\n";
                oss << "      \"FogInscatteringColorG\" : \"" << HeightFogData->FogInscatteringColor.G << "\"";

                oss << ",\n";
                oss << "      \"FogInscatteringColorB\" : \"" << HeightFogData->FogInscatteringColor.B << "\"";

                oss << ",\n";
                oss << "      \"FogInscatteringColorA\" : \"" << HeightFogData->FogInscatteringColor.A << "\"";
            }
        }
            else if (FireBallData)
            {
                oss << ",\n";
                oss << "      \"Color\" : [" << FireBallData->Color.R << ", " << FireBallData->Color.G << ", " << FireBallData->Color.B << ", " << FireBallData->Color.A << "]";
                oss << ",\n";
                oss << "      \"Intensity\" : \"" << FireBallData->Intensity << "\"";
                oss << ",\n";
                oss << "      \"Radius\" : \"" << FireBallData->Radius << "\"";
                oss << ",\n";
                oss << "      \"Falloff\" : \"" << FireBallData->FallOff << "\"";
            }
        }
        // 비계층 컴포넌트일때
        else
        {
            FRotationMovementComponentData* RotationMovementData = dynamic_cast<FRotationMovementComponentData*>(Component);
            FProjectileMovementComponentData* ProjectileMovementData = dynamic_cast<FProjectileMovementComponentData*>(Component);

            if (RotationMovementData)
            {
                oss << "      \"RotationAngleX\" : \"" << RotationMovementData->RotationAngle.X << "\"";
                oss << ",\n";
                oss << "      \"RotationAngleY\" : \"" << RotationMovementData->RotationAngle.Y << "\"";
                oss << ",\n";
                oss << "      \"RotationAngleZ\" : \"" << RotationMovementData->RotationAngle.Z << "\"";
            }
            else if (ProjectileMovementData)
            {
                oss << "      \"LaunchDirectionX\" : \"" << ProjectileMovementData->LaunchDirection.X << "\"";
                oss << ",\n";
                oss << "      \"LaunchDirectionY\" : \"" << ProjectileMovementData->LaunchDirection.Y << "\"";
                oss << ",\n";
                oss << "      \"LaunchDirectionZ\" : \"" << ProjectileMovementData->LaunchDirection.Z << "\"";
            }
        }

        oss << "\n";
        oss << "    }" << (i + 1 < SceneData.Components.size() ? "," : "") << "\n";
    }
    oss << "  ]\n";
    oss << "}\n";

    const std::string finalPath = outPath.make_preferred().string();
    std::ofstream OutFile(finalPath.c_str(), std::ios::out | std::ios::trunc);
    if (OutFile.is_open())
    {
        OutFile << oss.str();
        OutFile.close();
    }
    else
    {
        UE_LOG("Scene save failed. Cannot open file: %s", finalPath.c_str());
    }
}

FSceneData FSceneLoader::Load(const FString& FileName)
{
    FSceneData Result;

    std::ifstream file(FileName);
    if (!file.is_open())
    {
        UE_LOG("Scene load failed. Cannot open file: %s", FileName.c_str());
        return Result;
    }

    std::stringstream Buffer;
    Buffer << file.rdbuf();
    std::string content = Buffer.str();

    try {
        JSON j = JSON::Load(content);
        Result = Parse(j);
    }
    catch (const std::exception& e) {
        UE_LOG("Scene load failed. JSON parse error: %s", e.what());
    }

    return Result;
}

FSceneData FSceneLoader::Parse(const JSON& Json)
{
    FSceneData SceneData;

    // Version
    if (Json.hasKey("Version"))
        SceneData.Version = static_cast<uint32>(Json.at("Version").ToInt());

    // NextUUID
    if (Json.hasKey("NextUUID"))
        SceneData.NextUUID = static_cast<uint32>(Json.at("NextUUID").ToInt());

    // Camera
    if (Json.hasKey("PerspectiveCamera"))
    {
        ParsePerspectiveCamera(Json, SceneData.Camera);
    }

    // Actor
    if (Json.hasKey("Actors"))
    {
        const JSON& ActorsJson = Json.at("Actors");
        for (size_t i = 0; i < ActorsJson.size(); ++i)
        {
            const JSON& ActorJson = ActorsJson.at(i);
            FActorData Actor;

            if (ActorJson.hasKey("UUID"))
                Actor.UUID = static_cast<uint32>(ActorJson.at("UUID").ToInt());
            if (ActorJson.hasKey("Type"))
                Actor.Type = ActorJson.at("Type").ToString();
            if (ActorJson.hasKey("Name"))
                Actor.Name = ActorJson.at("Name").ToString();
            if (ActorJson.hasKey("RootComponentUUID"))
                Actor.RootComponentUUID = static_cast<uint32>(ActorJson.at("RootComponentUUID").ToInt());

            SceneData.Actors.push_back(Actor);
        }
    }

    // Components
    if (Json.hasKey("Components"))
    {
        const JSON& CompsJson = Json.at("Components");
        for (size_t i = 0; i < CompsJson.size(); ++i)
        {
            const JSON& CompJson = CompsJson.at(i);
            FComponentData* ComponentData = nullptr;

            // Type 별 속성 처리
            if (!CompJson.hasKey("Type"))
                continue;

            if (CompJson.at("Type").ToString() == "UStaticMeshComponent")
            {
                ComponentData = new FStaticMeshComponentData;

                FStaticMeshComponentData* StaticMeshData = dynamic_cast<FStaticMeshComponentData*>(ComponentData);

                // Type별 속성
                if (CompJson.hasKey("StaticMesh"))
                    StaticMeshData->StaticMesh = CompJson.at("StaticMesh").ToString();

                if (CompJson.hasKey("Materials"))
                {
                    const JSON& matsJson = CompJson.at("Materials");
                    for (size_t m = 0; m < matsJson.size(); ++m)
                    {
                        StaticMeshData->Materials.push_back(matsJson.at(m).ToString());
                    }
                }
            }
            else if (CompJson.at("Type").ToString() == "UBillboardComponent")
            {
                ComponentData = new FBillboardComponentData;

                FBillboardComponentData* BillboardData = dynamic_cast<FBillboardComponentData*>(ComponentData);

                // Type별 속성
                if (CompJson.hasKey("Texture"))
                    BillboardData->Texture = CompJson.at("Texture").ToString();
            }
            else if (CompJson.at("Type").ToString() == "UTextRenderComponent")
            {
                ComponentData = new FTextComponentData;

                FTextComponentData* TextData = dynamic_cast<FTextComponentData*>(ComponentData);

                // Type별 속성
                if (CompJson.hasKey("Text"))
                    TextData->Text = CompJson.at("Text").ToString();
            }
            else if (CompJson.at("Type").ToString() == "UDecalComponent")
            {
                ComponentData = new FDecalComponentData;

                FDecalComponentData* DecalData = dynamic_cast<FDecalComponentData*>(ComponentData);

                // Type별 속성
                if (CompJson.hasKey("Texture"))
                    DecalData->Texture = CompJson.at("Texture").ToString();
                if (CompJson.hasKey("FadeType"))
                    DecalData->FadeType = stoi(CompJson.at("FadeType").ToString());
                if (CompJson.hasKey("Duration"))
                    DecalData->Duration = stof(CompJson.at("Duration").ToString());
                if (CompJson.hasKey("Min"))
                    DecalData->Min = stof(CompJson.at("Min").ToString());
                if (CompJson.hasKey("Max"))
                    DecalData->Max = stof(CompJson.at("Max").ToString());
                if (CompJson.hasKey("Alpha"))
                    DecalData->Alpha = stof(CompJson.at("Alpha").ToString());
                if (CompJson.hasKey("FadeEnabled"))
                    DecalData->bIsFadeEnabled = CompJson.at("FadeEnabled").ToString() == "true";
                if (CompJson.hasKey("FadeStart"))
                    DecalData->bIsFadeStart = CompJson.at("FadeStart").ToString() == "true";
                if (CompJson.hasKey("Loop"))
                    DecalData->bIsLoop = CompJson.at("Loop").ToString() == "true";
                if (CompJson.hasKey("ElapsedTime"))
                    DecalData->ElapsedTime = stof(CompJson.at("ElapsedTime").ToString());
            }
            else if (CompJson.at("Type").ToString() == "UHeightFogComponent")
            {
                ComponentData = new FHeightFogComponentData;

                FHeightFogComponentData* HeightFogData = dynamic_cast<FHeightFogComponentData*>(ComponentData);

                // Type별 속성
                if (CompJson.hasKey("FogDensity"))
                    HeightFogData->FogDensity = stof(CompJson.at("FogDensity").ToString());
                if (CompJson.hasKey("FogHeightFalloff"))
                    HeightFogData->FogHeightFalloff = stof(CompJson.at("FogHeightFalloff").ToString());
                if (CompJson.hasKey("StartDistance"))
                    HeightFogData->StartDistance = stof(CompJson.at("StartDistance").ToString());
                if (CompJson.hasKey("FogCutoffDistance"))
                    HeightFogData->FogCutoffDistance = stof(CompJson.at("FogCutoffDistance").ToString());
                if (CompJson.hasKey("FogMaxOpacity"))
                    HeightFogData->FogMaxOpacity = stof(CompJson.at("FogMaxOpacity").ToString());

                // FLinearColor 복원
                if (CompJson.hasKey("FogInscatteringColorR"))
                    HeightFogData->FogInscatteringColor.R = stof(CompJson.at("FogInscatteringColorR").ToString());
                if (CompJson.hasKey("FogInscatteringColorG"))
                    HeightFogData->FogInscatteringColor.G = stof(CompJson.at("FogInscatteringColorG").ToString());
                if (CompJson.hasKey("FogInscatteringColorB"))
                    HeightFogData->FogInscatteringColor.B = stof(CompJson.at("FogInscatteringColorB").ToString());
                if (CompJson.hasKey("FogInscatteringColorA"))
                    HeightFogData->FogInscatteringColor.A = stof(CompJson.at("FogInscatteringColorA").ToString());
            }
            else if (CompJson.at("Type").ToString() == "UFireBallComponent")
            {
                ComponentData = new FFireBallComponentData;
                FFireBallComponentData* FireBallData = dynamic_cast<FFireBallComponentData*>(ComponentData);

                if (CompJson.hasKey("Color"))
                {
                    auto colorArr = CompJson.at("Color");
                    FireBallData->Color = FLinearColor(
                        (float)colorArr[0].ToFloat(),
                        (float)colorArr[1].ToFloat(),
                        (float)colorArr[2].ToFloat(),
                        (float)colorArr[3].ToFloat()
                    );
                }
                if (CompJson.hasKey("Intensity"))
                    FireBallData->Intensity = stof(CompJson.at("Intensity").ToString());
                if (CompJson.hasKey("Radius"))
                    FireBallData->Radius = stof(CompJson.at("Radius").ToString());
                if (CompJson.hasKey("Falloff"))
                    FireBallData->FallOff = stof(CompJson.at("Falloff").ToString());
            }
            else if (CompJson.at("Type").ToString() == "URotationMovementComponent")
            {
                ComponentData = new FRotationMovementComponentData;

                FRotationMovementComponentData* RMCData = dynamic_cast<FRotationMovementComponentData*>(ComponentData);
                if (CompJson.hasKey("RotationAngleX"))
                {
                    RMCData->RotationAngle.X = stof(CompJson.at("RotationAngleX").ToString());
                }
                if (CompJson.hasKey("RotationAngleY"))
                {
                    RMCData->RotationAngle.Y = stof(CompJson.at("RotationAngleY").ToString());
                }
                if (CompJson.hasKey("RotationAngleZ"))
                {
                    RMCData->RotationAngle.Z = stof(CompJson.at("RotationAngleZ").ToString());
                }
            }
            else if (CompJson.at("Type").ToString() == "UProjectileMovementComponent")
            {
                ComponentData = new FProjectileMovementComponentData;

                FProjectileMovementComponentData* PMCData = dynamic_cast<FProjectileMovementComponentData*>(ComponentData);
                if (CompJson.hasKey("LaunchDirectionX"))
                {
                    PMCData->LaunchDirection.X = stof(CompJson.at("LaunchDirectionX").ToString());
                }
                if (CompJson.hasKey("LaunchDirectionY"))
                {
                    PMCData->LaunchDirection.Y = stof(CompJson.at("LaunchDirectionY").ToString());
                }
                if (CompJson.hasKey("LaunchDirectionZ"))
                {
                    PMCData->LaunchDirection.Z = stof(CompJson.at("LaunchDirectionZ").ToString());
                }
            }
            else
            {
                continue;
            }


            // 공통 속성 처리
            if (CompJson.hasKey("UUID"))
                ComponentData->UUID = static_cast<uint32>(CompJson.at("UUID").ToInt());
            if (CompJson.hasKey("OwnerActorUUID"))
                ComponentData->OwnerActorUUID = static_cast<uint32>(CompJson.at("OwnerActorUUID").ToInt());
            if (CompJson.hasKey("Type"))
                ComponentData->Type = CompJson.at("Type").ToString();
            // 계층 컴포넌트에만 있는 속성을 처리
            if (CompJson.hasKey("IsHierarchical"))
                ComponentData->IsHierarchical = (CompJson.at("IsHierarchical").ToString() == "true");

            if (ComponentData->IsHierarchical)
            {
                FSceneComponentData* SceneComponentData = dynamic_cast<FSceneComponentData*>(ComponentData);
            
                if (CompJson.hasKey("ParentComponentUUID"))
                    SceneComponentData->ParentComponentUUID = static_cast<uint32>(CompJson.at("ParentComponentUUID").ToInt());
            
                // Transform
                if (CompJson.hasKey("RelativeLocation"))
                {
                    auto loc = CompJson.at("RelativeLocation");
                    SceneComponentData->RelativeLocation = FVector(
                        (float)loc[0].ToFloat(),
                        (float)loc[1].ToFloat(),
                        (float)loc[2].ToFloat()
                    );
                }

                if (CompJson.hasKey("RelativeRotation"))
                {
                    auto rot = CompJson.at("RelativeRotation");
                    SceneComponentData->RelativeRotation = FVector(
                        (float)rot[0].ToFloat(),
                        (float)rot[1].ToFloat(),
                        (float)rot[2].ToFloat()
                    );
                }

                if (CompJson.hasKey("RelativeScale"))
                {
                    auto scale = CompJson.at("RelativeScale");
                    SceneComponentData->RelativeScale = FVector(
                        (float)scale[0].ToFloat(),
                        (float)scale[1].ToFloat(),
                        (float)scale[2].ToFloat()
                    );
                }
            }

            SceneData.Components.push_back(ComponentData);
        }
    }

    return SceneData;
}

// ─────────────────────────────────────────────
// NextUUID 메타만 읽어오는 간단한 헬퍼
// 저장 포맷상 "NextUUID"는 "마지막으로 사용된 UUID"이므로,
// 호출 측에서 +1 해서 SetNextUUID 해야 함
// ─────────────────────────────────────────────
bool FSceneLoader::TryReadNextUUID(const FString& FilePath, uint32& OutNextUUID)
{
    std::ifstream file(FilePath);
    if (!file.is_open())
    {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    try
    {
        JSON j = JSON::Load(content);
        if (j.hasKey("NextUUID"))
        {
            // 정수 파서가 없으면 ToFloat로 받아서 캐스팅
			OutNextUUID = static_cast<uint32>(j.at("NextUUID").ToInt());
            return true;
        }
    }
    catch (...)
    {
        // 무시하고 false 반환
    }
    return false;
}
