#include "pch.h"
#include "SelectionManager.h"
#include "Picking.h"
#include "SceneLoader.h"
#include "CameraActor.h"
#include "StaticMeshActor.h"
#include "CameraComponent.h"
#include "ObjectFactory.h"
#include "TextRenderComponent.h"
#include "AABoundingBoxComponent.h"
#include "FViewport.h"
#include "SViewportWindow.h"
#include "SMultiViewportWindow.h"
#include "StaticMesh.h"
#include "ObjManager.h"
#include "SceneRotationUtils.h"
#include "Frustum.h"
#include "Octree.h"
#include "BVH.h"
#include "UEContainer.h"
#include "DecalComponent.h"
#include "PickingTimer.h"
#include "RenderingStats.h"
#include "UI/StatsOverlayD2D.h"
#include "PrimitiveComponent.h"
#include "HeightFogComponent.h"
#include "HeightFog.h"
#include "RotationMovementComponent.h"
#include "ProjectileMovementComponent.h"
#include "FireBallComponent.h"

extern float CLIENTWIDTH;
extern float CLIENTHEIGHT;

static inline FString GetBaseNameNoExt(const FString& Path)
{
    const size_t sep = Path.find_last_of("/\\");
    const size_t start = (sep == FString::npos) ? 0 : sep + 1;

    const FString ext = ".obj";
    size_t end = Path.size();
    if (end >= ext.size() && Path.compare(end - ext.size(), ext.size(), ext) == 0)
    {
        end -= ext.size();
    }
    if (start <= end) return Path.substr(start, end - start);
    return Path;
}


UWorld::UWorld() : ResourceManager(UResourceManager::GetInstance())
                   , UIManager(UUIManager::GetInstance())
                   , InputManager(UInputManager::GetInstance())
                   , SelectionManager(USelectionManager::GetInstance())
                   , BVH(nullptr)
{
    Level = NewObject<ULevel>();
}

UWorld::~UWorld()
{
    // Level의 Actors 정리 (PIE는 복제된 액터들만 삭제)
    if (Level)
    {
        for (AActor* Actor : Level->GetActors())
        {
            ObjectFactory::DeleteObject(Actor);
        }

        // Level 자체 정리
        ObjectFactory::DeleteObject(Level);
        Level = nullptr;
    }

    // PIE 월드가 아닐 때만 공유 리소스 삭제
    if (WorldType == EWorldType::Editor)
    {
        // 카메라 정리
        ObjectFactory::DeleteObject(MainCameraActor);
        MainCameraActor = nullptr;

        // Grid 정리
        ObjectFactory::DeleteObject(GridActor);
        GridActor = nullptr;

        // GizmoActor 정리
        ObjectFactory::DeleteObject(GizmoActor);
        GizmoActor = nullptr;

        // BVH 정리
        if (BVH)
        {
            delete BVH;
            BVH = nullptr;
        }

        // ObjManager 정리
        FObjManager::Clear();
    }
    else if (WorldType == EWorldType::PIE)
    {
        // PIE 월드는 공유 포인터만 nullptr로 설정 (삭제하지 않음)
        MainCameraActor = nullptr;
        GridActor = nullptr;
        GizmoActor = nullptr;
        BVH = nullptr;
        Renderer = nullptr;
        MainViewport = nullptr;
        MultiViewport = nullptr;
    }
}

static void DebugRTTI_UObject(UObject* Obj, const char* Title)
{
    if (!Obj)
    {
        UE_LOG("[RTTI] Obj == null\r\n");
        return;
    }

    char buf[256];
    UE_LOG("========== RTTI CHECK ==========\r\n");
    if (Title)
    {
        std::snprintf(buf, sizeof(buf), "[RTTI] %s\r\n", Title);
        UE_LOG(buf);
    }

    // 1) 현재 동적 타입 이름
    std::snprintf(buf, sizeof(buf), "[RTTI] TypeName = %s\r\n", Obj->GetClass()->Name);
    UE_LOG(buf);

    // 2) IsA 체크 (파생 포함)
    std::snprintf(buf, sizeof(buf), "[RTTI] IsA<AActor>      = %d\r\n", (int)Obj->IsA<AActor>());
    UE_LOG(buf);
    std::snprintf(buf, sizeof(buf), "[RTTI] IsA<ACameraActor> = %d\r\n",
                  (int)Obj->IsA<ACameraActor>());
    UE_LOG(buf);

    //// 3) 정확한 타입 비교 (파생 제외)
    //std::snprintf(buf, sizeof(buf), "[RTTI] EXACT ACameraActor = %d\r\n",
    //    (int)(Obj->GetClass() == ACameraActor::StaticClass()));
    //UE_LOG(buf);

    // 4) 상속 체인 출력
    UE_LOG("[RTTI] Inheritance chain: ");
    for (const UClass* c = Obj->GetClass(); c; c = c->Super)
    {
        std::snprintf(buf, sizeof(buf), "%s%s", c->Name, c->Super ? " <- " : "\r\n");
        UE_LOG(buf);
    }
    //FString Name = Obj->GetName();
    std::snprintf(buf, sizeof(buf), "[RTTI] TypeName = %s\r\n", Obj->GetName().ToString().c_str());
    OutputDebugStringA(buf);
    OutputDebugStringA("================================\r\n");
}

void UWorld::Initialize()
{
    FObjManager::Preload();

    // 새 씬 생성
    CreateNewScene();

    InitializeMainCamera();
    InitializeGrid();
    InitializeGizmo();


    // 액터 간 참조 설정
    SetupActorReferences();
}

void UWorld::InitializeMainCamera()
{
    MainCameraActor = NewObject<ACameraActor>();

    DebugRTTI_UObject(MainCameraActor, "MainCameraActor");
    UIManager.SetCamera(MainCameraActor);

    EngineActors.Add(MainCameraActor);
}

void UWorld::InitializeGrid()
{
    GridActor = NewObject<AGridActor>();
    GridActor->Initialize();

    // Add GridActor to Actors array so it gets rendered in the main loop
    EngineActors.push_back(GridActor);
    //EngineActors.push_back(GridActor);
}

void UWorld::InitializeGizmo()
{
    // === 기즈모 엑터 초기화 ===
    GizmoActor = NewObject<AGizmoActor>();
    GizmoActor->SetWorld(this);
    GizmoActor->SetActorTransform(FTransform(FVector{0, 0, 0},
                                             FQuat::MakeFromEuler(FVector{0, -90, 0}),
                                             FVector{1, 1, 1}));
    // 기즈모에 카메라 참조 설정
    if (MainCameraActor)
    {
        GizmoActor->SetCameraActor(MainCameraActor);
    }

    UIManager.SetGizmoActor(GizmoActor);
}

/**
* @brief BVH 생성
*/
void UWorld::InitializeSceneGraph(TArray<AActor*>& Actors)
{
    TArray<UPrimitiveComponent*> Primitives;
    for (AActor* Actor : Actors)
    {
        if (Actor)
        {
            for (UActorComponent* Component : Actor->GetComponents())
            {
                if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
                {
                    Primitives.Add(Primitive);
                }
            }
        }
    }

    // BVH 초기화 및 빌드
    if (!BVH)
    {
        BVH = new FBVH();
    }
    BVH->Build(Primitives);
}

void UWorld::RenderSceneGraph()
{
    if (!Octree)
    {
        return;
    }
    Octree->Render(nullptr);
}

void UWorld::SetRenderer(URenderer* InRenderer)
{
    Renderer = InRenderer;
}

void UWorld::Render()
{    
    Renderer->BeginFrame();
    UIManager.Render();

    // UIManager의 뷰포트 전환 상태에 따라 렌더링 변경 SWidget으로 변경해줄거임

    if (MultiViewport)
    {
        MultiViewport->OnRender();
    }
    Renderer->RenderPostProcess();
    //프레임 종료 
    UIManager.EndFrame();
    Renderer->EndFrame();
}

void UWorld::RenderViewports(ACameraActor* Camera, FViewport* Viewport)
{
    if (Viewport->GetSizeX() != 0 && Viewport->GetSizeY() != 0)
    {
        bIsFXAAEnabled = Viewport->IsShowFlagEnabled(EEngineShowFlags::SF_FXAA);
        FVector4 ViewportRect = {
            static_cast<float>(Viewport->GetStartX()),
            static_cast<float>(Viewport->GetStartY()),
            static_cast<float>(Viewport->GetSizeX()),
            static_cast<float>(Viewport->GetSizeY())
        };
        FXAAData.ViewportRect = ViewportRect;
        FXAAData.Pad = {};
        Renderer->SetOffscreenRenderTarget(FXAAData, bIsFXAAEnabled);
        if (bIsFXAAEnabled)
        {
            Renderer->GetRHIDevice()->OMSetRenderTargetToOffscreen();
            Renderer->GetRHIDevice()->ClearOffscreenBackBuffer();
        }
    }
   
    
    // 뷰포트의 실제 크기로 aspect ratio 계산
    float ViewportAspectRatio = static_cast<float>(Viewport->GetSizeX()) / static_cast<float>(
        Viewport->GetSizeY());
    if (Viewport->GetSizeY() == 0)
    {
        ViewportAspectRatio = 1.0f;
    } // 0으로 나누기 방지

    FMatrix ViewMatrix = Camera->GetViewMatrix();
    FMatrix ProjectionMatrix = Camera->GetProjectionMatrix(ViewportAspectRatio, Viewport);
    if (!Renderer)
    {
        return;
    }
    FVector rgb(1.0f, 1.0f, 1.0f);

    FFrustum ViewFrustum;
    ViewFrustum.Update(ViewMatrix * ProjectionMatrix);


    Renderer->BeginLineBatch();
    Renderer->SetViewModeType(ViewModeIndex);

    int AllActorCount = 0;
    int FrustumCullCount = 0;

    const TArray<AActor*>& LevelActors = Level ? Level->GetActors() : TArray<AActor*>();

    // 렌더러의 이전 프레임 데이터 제거
    Renderer->ClearFireBallData();

    // Pass 2를 위해 Decal을 미리 저장하기 위한 컨테이너
    TArray<UDecalComponent*> DecalVolumes;

    // Pass 2를 위해 StaticMesh를 미리 저장하기 위한 컨테이너
    TArray<UStaticMeshComponent*> StaticMeshes;

    // Pass 3를 위해 Fog를 별도로 저장하기 위한 컨테이너
    TArray<UHeightFogComponent*> HeightFogs;

    // Base Pass에서 그릴 Primitives
    TArray<UPrimitiveComponent*> BasePassPrimitives; 

    // Pass 1: 데칼을 제외한 모든 오브젝트 렌더링 (Depth 버퍼 채우기)
    for (AActor* Actor : LevelActors)
    {
        // 일반 액터들 렌더링
        if (!Viewport->IsShowFlagEnabled(EEngineShowFlags::SF_Primitives))
        {
            continue;
        }
        if (!Actor)
        {
            continue;
        }
        if (Actor->GetActorHiddenInGame())
        {
            continue;
        }
        if (Cast<AStaticMeshActor>(Actor) &&
            !Viewport->IsShowFlagEnabled(EEngineShowFlags::SF_StaticMeshes))
        {
            continue;
        }
        AllActorCount++;
        for (UActorComponent* Component : Actor->GetComponents())
        {
            if (!Component)
            {
                continue;
            }

            if (UActorComponent* ActorComp = Cast<UActorComponent>(Component))
            {
                if (!ActorComp->IsActive())
                {
                    continue;
                }
            }

            if (Cast<UTextRenderComponent>(Component) &&
                !Viewport->IsShowFlagEnabled(EEngineShowFlags::SF_BillboardText))
            {
                continue;
            }

            if (Cast<UAABoundingBoxComponent>(Component) &&
                !Viewport->IsShowFlagEnabled(EEngineShowFlags::SF_BoundingBoxes))
            {
                continue;
            }

            UDecalComponent* DecalComponent = Cast<UDecalComponent>(Component);
            // Decal Show Flag가 켜져 있다면
            if (Viewport->IsShowFlagEnabled(EEngineShowFlags::SF_Decal))
            {
                // Pass 2를 위해 Decal을 별도로 저장
                if (DecalComponent)
                {
                    DecalVolumes.Push(DecalComponent);
                }
            }
            else
            {
                // Decal Show Flag가 꺼져있으면 DecalComponent는 무시
                if (DecalComponent)
                    continue;
            }

            // StaticMeshComponent는 데칼과 파이어볼 모두 사용하므로 플래그와 무관하게 항상 수집
            
            if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component))
            {
                StaticMeshes.Push(StaticMeshComponent);
            }

            if (UHeightFogComponent* HeightFog = Cast<UHeightFogComponent>(Component))
            {
                HeightFogs.Add(HeightFog);
                continue;
            }
            // FireBall Pass를 위해 FireBall 데이터 수집
            if (UFireBallComponent* FireBallComponent = Cast<UFireBallComponent>(Component) )
            {
                {
                    if (Viewport->IsShowFlagEnabled(EEngineShowFlags::SF_DebugLines))
                    {
                        FireBallComponent->AddDebugLine(Renderer);
                    }
                    // 실제로 그리는게 아닌 정보 수집
                    FireBallComponent->Render(Renderer, ViewMatrix, ProjectionMatrix);
                    continue;
                }
            }
            if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
            {
                bool bIsSelected = SelectionManager.IsActorSelected(Actor);

                //// 선택된 액터는 항상 앞에 보이도록 depth test를 Always로 설정
                //if (bIsSelected)//나중에 추가구현
                //{
                //    Renderer->OMSetDepthStencilState(EComparisonFunc::Always);
                //}

                Renderer->UpdateHighLightConstantBuffer(bIsSelected, rgb, 0, 0, 0, 0);
                Primitive->Render(Renderer, ViewMatrix, ProjectionMatrix);

                //// depth test 원래대로 복원
                //if (bIsSelected)
                //{
                //    Renderer->OMSetDepthStencilState(EComparisonFunc::LessEqual);
                //}
            }
        }
        Renderer->OMSetBlendState(false);
    }

    // 엔진 액터들 (그리드 등) 렌더링
    RenderEngineActors(ViewMatrix, ProjectionMatrix, Viewport);

    URenderingStatsCollector& StatsCollector =
        URenderingStatsCollector::GetInstance();

    if (Viewport->IsShowFlagEnabled(EEngineShowFlags::SF_Decal))
    {

        TStatId ViewportAspectDecalRenderStatId;
        FScopeCycleCounter ViewportAspectDecalRenderTimer(ViewportAspectDecalRenderStatId);
        
        // 추가 통계용
        int32 TotalPrimitivesInScene = BVH ? BVH->GetPrimitiveCount() : StaticMeshes.Num();
        //int32 TotalPrimitivesInScene = StaticMeshes.Num();
        int32 TotalCandidatePrimitives = 0;
        int32 FinalDrawCalls = 0;


        // Pass 2: 데칼 렌더링 (Depth 버퍼를 읽어서 다른 오브젝트 위에 투영)
        if (BVH)
        {
            // 각 데칼 볼륨에 대해 수행
            for (UDecalComponent* DecalVolume : DecalVolumes)
            {
                UOBoundingBoxComponent* DecalOBB = DecalVolume->GetOBBComponent();
                if (!DecalOBB)
                {
                    continue;
                }
                // ======= Broad Phase =======
                // Decal OBB를 감싸는 AABB
                FBound DecalWorldAABB = DecalOBB->GetWorldBound();
                
                // BVH에 AABB 쿼리 -> 후보 프리미티브 목록 얻기
                TArray<UPrimitiveComponent*> CandidatePrimitives;
                BVH->IntersectAABB(DecalWorldAABB, CandidatePrimitives);
                // 스태틱 메시 개수만 세기 위해 임시 처리
               /* for (UPrimitiveComponent* CandidatePrimitive : CandidatePrimitives)
                {
                    if (UStaticMeshComponent* CandidateStaticMeshComp = Cast<UStaticMeshComponent>(CandidatePrimitive))
                    {
                        TotalCandidatePrimitives++;
                    }
                }*/
                // AABB, OBB 컴포넌트는 포함 안되어있음(BVH에 없기 때문)
                TotalCandidatePrimitives += CandidatePrimitives.Num();
                // ======= Narrow Phase =======
                // 후보군 프리미티브에서 SAT 검사 수행
                for (UPrimitiveComponent* CandidatePrimitive : CandidatePrimitives)
                {
                    // 후보군 프리미티브에서 StaticMeshComponent 뽑기
                    UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(CandidatePrimitive);
                    if (!StaticMeshComponent)
                    {
                        continue;
                    }

                    // 데칼 OBB와 StaticMesh의 AABB와 충돌 검사(SAT)
                    if (DecalOBB->IntersectsWithAABB(StaticMeshComponent->GetWorldBound()))
                    {
                        FinalDrawCalls++; // 최종 드로우 콜 수 증가
                        DecalVolume->UpdateFade(DeltaSeconds);
                        DecalVolume->ProjectDecal(
                            Renderer,
                            StaticMeshComponent,
                            ViewMatrix,
                            ProjectionMatrix
                        );
                    }
                }
            }
        }
        else // BVH가 없는 경우, 기존 방식으로 렌더링
        {
            for (UDecalComponent* DecalVolume : DecalVolumes)
            {
                for (UStaticMeshComponent* StaticMesh : StaticMeshes)
                {
                    DecalVolume->ProjectDecal(
                        Renderer,
                        StaticMesh,
                        ViewMatrix,
                        ProjectionMatrix
                    );
                }
            }
            FinalDrawCalls = DecalVolumes.size() * StaticMeshes.size();
        }
        

        uint64_t ViewportAspectCycleDiff = ViewportAspectDecalRenderTimer.Finish();
        double ViewportAspectDecalRenderTimeMs = FPlatformTime::ToMilliseconds(ViewportAspectCycleDiff);
        
        StatsCollector.UpdateDecalStats(
            DecalVolumes.size(),
            (float)ViewportAspectDecalRenderTimeMs
        );
        StatsCollector.UpdateDecalCullingStats(
            TotalPrimitivesInScene,
            TotalCandidatePrimitives,
            FinalDrawCalls
        );
    }
    else
    {
        StatsCollector.UpdateDecalStats(0, 0.0f);
    }

    UStatsOverlayD2D::Get().UpdateDecalStats(
        StatsCollector.GetDecalNum(),
        StatsCollector.GetDecalRenderTimeTotal(),
        StatsCollector.GetDecalTotalPrimitives(),
        StatsCollector.GetDecalCandidatePrimitives(),
        StatsCollector.GetDecalFinalDrawCalls());

    // BVH 디버그 렌더링
    if (BVH && Viewport->IsShowFlagEnabled(EEngineShowFlags::SF_BoundingBoxes) && WorldType != EWorldType::PIE)
    {
        BVH->Render(Renderer);
    }

    // Pass 3 - Fog 구현을 위한 Screen Aligned Quad Render 구현
    // Update viewport buffer for correct UV mapping in multi-viewport setup
    Renderer->UpdateViewportBuffer(
        static_cast<float>(Viewport->GetStartX()),
        static_cast<float>(Viewport->GetStartY()),
        static_cast<float>(Viewport->GetSizeX()),
        static_cast<float>(Viewport->GetSizeY())
    );

    if (Viewport->IsShowFlagEnabled(EEngineShowFlags::SF_Fog))
    {
        for (const auto& HeightFog : HeightFogs)
        {
            HeightFog->Render(Renderer, ViewMatrix, ProjectionMatrix);
        }
    }
    Renderer->EndLineBatch(FMatrix::Identity(), ViewMatrix, ProjectionMatrix);

    // FireBall Pass
    const TArray<FireBallBufferType>& FireBallDatas = Renderer->GetFrameFireBallData();
    if (!FireBallDatas.empty() && Viewport->IsShowFlagEnabled(EEngineShowFlags::SF_FireBall))
    {
        // Fireball Pass를 위한 프로젝션 행렬을 약간 수정하여 Z-fighting 방지
        FMatrix FireballProjectionMatrix = ProjectionMatrix;
        // Z-fighting을 해결하기 위한 작은 오프셋 값. 
        // 행렬의 3행 3열과 4행 3열을 수정하여 z값을 미세하게 줄여(카메라에 가깝게 만듦) 
        // 따라서 깊이 테스트를 통과가능
        float epsilon = 0.0001f; 
        FireballProjectionMatrix.M[2][2] *= (1.0f - epsilon);
        FireballProjectionMatrix.M[3][2] *= (1.0f - epsilon);

        // 가산 블렌딩
        Renderer->OMSetBlendState(EBlendMode::Addicitve);
        // 깊이 테스트: LESSEQUAL, 깊이 쓰기 비활성화
        Renderer->OMSetDepthStencilState(EComparisonFunc::LessEqualReadOnly); 
        Renderer->RSSetDefaultState();

        UShader* FireBalllShader = ResourceManager.Load<UShader>("FireBallShader.hlsl");
        Renderer->PrepareShader(FireBalllShader);

        for (const FireBallBufferType& FireBallData : FireBallDatas)
        {
            Renderer->UpdateFireBallConstantBuffer(FireBallData);
            // 수집된 StaticMeshCompnent에 대해 한번 더 그리기
            for (UStaticMeshComponent* StaticMeshComponent : StaticMeshes)
            {
                if (StaticMeshComponent && StaticMeshComponent->IsActive())
                {
                    // YUptoZUp이 반영되지 않은 World Matrix를 보낸다.
                    Renderer->UpdateRealWorldBuffer(StaticMeshComponent->GetRealWorldMatrix());

                    // 수정된 프로젝션 행렬을 사용하여 렌더링
                    Renderer->UpdateConstantBuffer(StaticMeshComponent->GetWorldMatrix(), ViewMatrix, FireballProjectionMatrix);
                    Renderer->DrawSimpleMesh(StaticMeshComponent->GetStaticMesh());
                }
            }
        }
    }
    // 렌더 상태 복원
    Renderer->OMSetBlendState(false); // 블렌딩 비활성화
    Renderer->OMSetDepthStencilState(EComparisonFunc::LessEqual); // 기본 깊이 스텐실 상태로 복원
    Renderer->RSSetDefaultState();

    // Pass 4 - View Mode : SceneDepth가 활성화되어있는 경우 실행

    if (ViewModeIndex == EViewModeIndex::VMI_SceneDepth)
    {
        Renderer->RenderSceneDepth(Camera->GetCameraNear(), Camera->GetCameraFar());
    }
}

void UWorld::RenderEngineActors(const FMatrix& ViewMatrix, const FMatrix& ProjectionMatrix, FViewport* Viewport)
{
    for (AActor* EngineActor : EngineActors)
    {
        if (!EngineActor)
        {
            continue;
        }

        if (EngineActor->GetActorHiddenInGame())
        {
            continue;
        }

        if (Cast<AGridActor>(EngineActor) && !Viewport->IsShowFlagEnabled(EEngineShowFlags::SF_Grid))
        {
            continue;
        }

        for (UActorComponent* Component : EngineActor->GetComponents())
        {
            if (!Component)
            {
                continue;
            }

            if (UActorComponent* ActorComp = Cast<UActorComponent>(Component))
            {
                if (!ActorComp->IsActive())
                {
                    continue;
                }
            }

            if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
            {
                Renderer->SetViewModeType(ViewModeIndex);
                Primitive->Render(Renderer, ViewMatrix, ProjectionMatrix);
                Renderer->OMSetDepthStencilState(EComparisonFunc::LessEqual);
            }
        }
        Renderer->OMSetBlendState(false);
    }
}

void UWorld::Tick(float DeltaSeconds)
{
    // Level의 Actors Tick
    if (Level && WorldType == EWorldType::PIE)
    {
        for (AActor* Actor : Level->GetActors())
        {
            if (Actor && Actor->IsActorTickEnabled())
            {
                Actor->Tick(DeltaSeconds);
            }
        }
    }
    if (BVH)
    {
        // 액터 위치가 바뀐 경우에만 Refit
        if (bIsBVHDirty)
        {
            //UE_LOG("BVH Refit!");
            BVH->Refit();
            bIsBVHDirty = false;
        }
    }

    // Engine Actors Tick
    for (AActor* EngineActor : EngineActors)
    {
        if (EngineActor && EngineActor->IsActorTickEnabled())
        {
            EngineActor->Tick(DeltaSeconds);
        }
    }

    if (GizmoActor)
    {
        GizmoActor->Tick(DeltaSeconds);
    }

    //ProcessActorSelection();
    ProcessViewportInput();
    //Input Manager가 카메라 후에 업데이트 되어야함

    // 뷰포트 업데이트 - UIManager의 뷰포트 전환 상태에 따라
    if (MultiViewport)
    {
        MultiViewport->OnUpdate(DeltaSeconds);
    }

    //InputManager.Update();
    UIManager.Update(DeltaSeconds);
}

float UWorld::GetTimeSeconds() const
{
    return 0.0f;
}

bool UWorld::FrustumCullActors(const FFrustum& ViewFrustum, const AActor* Actor, int & FrustumCullCount)
{
    if (Actor->CollisionComponent)
    {
        FBound Test = Actor->CollisionComponent->GetWorldBoundFromCube();

        // 절두체 밖에 있다면, 이 액터의 렌더링 과정을 모두 건너뜁니다.
        if (!ViewFrustum.IsVisible(Test))
        {
            FrustumCullCount++;
            return true;
        }
    }
}

FString UWorld::GenerateUniqueActorName(const FString& ActorType)
{
    // Get current count for this type
    int32& CurrentCount = ObjectTypeCounts[ActorType];
    FString UniqueName = ActorType + "_" + std::to_string(CurrentCount);
    CurrentCount++;
    return UniqueName;
}

//
// 액터 제거
//
bool UWorld::DestroyActor(AActor* Actor)
{
    if (!Actor)
    {
        return false; // nullptr 들어옴 → 실패
    }

    // SelectionManager에서 선택 해제 (메모리 해제 전에 하자)
    USelectionManager::GetInstance().DeselectActor(Actor);

    // UIManager에서 픽된 액터 정리
    if (UIManager.GetPickedActor() == Actor)
    {
        UIManager.ResetPickedActor();
    }

    // 배열에서  제거 시도
    // Level에서 제거 시도
    if (Level)
    {
        Level->RemoveActor(Actor);

        // 메모리 해제
        ObjectFactory::DeleteObject(Actor);
        // 삭제된 액터 정리
        USelectionManager::GetInstance().CleanupInvalidActors();

        // 재빌드
        if (BVH)
        {
            RequestRebuildBVH();
        }

        return true; // 성공적으로 삭제
    }

    return false; // 월드에 없는 액터
}

inline FString ToObjFileName(const FString& TypeName)
{
    return "Data/" + TypeName + ".obj";
}

inline FString RemoveObjExtension(const FString& FileName)
{
    const FString Extension = ".obj";

    // 마지막 경로 구분자 위치 탐색 (POSIX/Windows 모두 지원)
    const uint64 Sep = FileName.find_last_of("/\\");
    const uint64 Start = (Sep == FString::npos) ? 0 : Sep + 1;

    // 확장자 제거 위치 결정
    uint64 End = FileName.size();
    if (End >= Extension.size() &&
        FileName.compare(End - Extension.size(), Extension.size(), Extension) == 0)
    {
        End -= Extension.size();
    }

    // 베이스 이름(확장자 없는 파일명) 반환
    if (Start <= End)
    {
        return FileName.substr(Start, End - Start);
    }

    // 비정상 입력 시 원본 반환 (안전장치)
    return FileName;
}

void UWorld::CreateNewScene()
{
    // Safety: clear interactions that may hold stale pointers
    SelectionManager.ClearSelection();
    UIManager.ResetPickedActor();
    // Level의 Actors 정리
    if (Level)
    {
        for (AActor* Actor : Level->GetActors())
        {
            ObjectFactory::DeleteObject(Actor);
        }
        Level->GetActors().clear();
    }

    if (Octree)
    {
        Octree->Release();//새로운 씬이 생기면 Octree를 지워준다.
    }
    if (BVH)
    {
        BVH->Clear();//새로운 씬이 생기면 BVH를 지워준다.
    }
    // 이름 카운터 초기화: 씬을 새로 시작할 때 각 BaseName 별 suffix를 0부터 다시 시작
    ObjectTypeCounts.clear();
}

// 액터 인터페이스 관리 메소드들
void UWorld::SetupActorReferences()
{
    if (GizmoActor && MainCameraActor)
    {
        GizmoActor->SetCameraActor(MainCameraActor);
    }
}

//마우스 피킹관련 메소드
void UWorld::ProcessActorSelection()
{
    if (InputManager.IsMouseButtonPressed(LeftButton))
    {
        const FVector2D MousePosition = UInputManager::GetInstance().GetMousePosition();
        {
            if (MultiViewport)
            {
                MultiViewport->OnMouseDown(MousePosition, 0);
            }
        }
    }
    if (InputManager.IsMouseButtonPressed(RightButton))
    {
        const FVector2D MousePosition = UInputManager::GetInstance().GetMousePosition();
        {
            if (MultiViewport)
            {
                MultiViewport->OnMouseDown(MousePosition, 0);
            }
        }
    }
    if (InputManager.IsMouseButtonPressed(RightButton))
    {
        const FVector2D MousePosition = UInputManager::GetInstance().GetMousePosition();
        {
            if (MultiViewport)
            {
                MultiViewport->OnMouseDown(MousePosition, 1);
            }
        }
    }
    if (InputManager.IsMouseButtonReleased(RightButton))
    {
        const FVector2D MousePosition = UInputManager::GetInstance().GetMousePosition();
        {
            if (MultiViewport)
            {
                MultiViewport->OnMouseUp(MousePosition, 1);
            }
        }
    }
}

void UWorld::ProcessViewportInput()
{
    const FVector2D MousePosition = UInputManager::GetInstance().GetMousePosition();

    if (InputManager.IsMouseButtonPressed(LeftButton))
    {
        const FVector2D MousePosition = UInputManager::GetInstance().GetMousePosition();
        {
            if (MultiViewport)
            {
                MultiViewport->OnMouseDown(MousePosition, 0);
            }
        }
    }
    if (InputManager.IsMouseButtonPressed(RightButton))
    {
        const FVector2D MousePosition = UInputManager::GetInstance().GetMousePosition();
        {
            if (MultiViewport)
            {
                MultiViewport->OnMouseDown(MousePosition, 1);
            }
        }
    }
    if (InputManager.IsMouseButtonReleased(LeftButton))
    {
        const FVector2D MousePosition = UInputManager::GetInstance().GetMousePosition();
        {
            if (MultiViewport)
            {
                MultiViewport->OnMouseUp(MousePosition, 0);
            }
        }
    }
    if (InputManager.IsMouseButtonReleased(RightButton))
    {
        const FVector2D MousePosition = UInputManager::GetInstance().GetMousePosition();
        {
            if (MultiViewport)
            {
                MultiViewport->OnMouseUp(MousePosition, 1);
            }
        }
    }
    MultiViewport->OnMouseMove(MousePosition);
}

void UWorld::SaveScene(const FString& SceneName)
{
    FSceneData SceneData;
    Serialize(&SceneData);

    // 카메라 데이터 채우기
    if (MainCameraActor && MainCameraActor->GetCameraComponent())
    {
        MainCameraActor->Serialize(&SceneData.Camera);
    }

    // Actor 및 Component 계층 수집
    for (AActor* Actor : Level->GetActors())
    {
        if (!Actor) continue;

        // Actor 데이터
        FActorData ActorData;
        Actor->Serialize(&ActorData);

        SceneData.Actors.push_back(ActorData);

        // OwnedComponents 순회
        for (UActorComponent* ActorComp : Actor->GetComponents())
        {
            if (!ActorComp) continue;

            UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(ActorComp);
            UDecalComponent* DecalComponent = Cast<UDecalComponent>(ActorComp);
            UBillboardComponent* BillboardComponent = Cast<UBillboardComponent>(ActorComp);
            UTextRenderComponent* TextComponent = Cast<UTextRenderComponent>(ActorComp);
            UFireBallComponent* FireBallComponent = Cast<UFireBallComponent>(ActorComp);

            UHeightFogComponent* HeightFogComponent = Cast<UHeightFogComponent>(ActorComp);
            // TODO : spotlight serialize

            FComponentData* ComponentData;

            if (StaticMeshComponent)
            {
                ComponentData = new FStaticMeshComponentData;
                StaticMeshComponent->Serialize(ComponentData);
            }
            else if (DecalComponent)
            {
                ComponentData = new FDecalComponentData;
                DecalComponent->Serialize(ComponentData);
            }
            else if (BillboardComponent)
            {
                ComponentData = new FBillboardComponentData;
                BillboardComponent->Serialize(ComponentData);
            }
            else if (TextComponent)
            {
                ComponentData = new FTextComponentData;
                TextComponent->Serialize(ComponentData);
            }
            else if (HeightFogComponent)
            {
                ComponentData = new FHeightFogComponentData;
                HeightFogComponent->Serialize(ComponentData);
            }
            else if (FireBallComponent)
            {
                ComponentData = new FFireBallComponentData;
                FireBallComponent->Serialize(ComponentData);
            }
            else
            {
                continue;
            }

            SceneData.Components.push_back(ComponentData);
        }

        // OwnedNonSceneComponents 순회
        for (UActorComponent* ActorComp : Actor->GetOwnedNonSceneComponent())
        {
            if (!ActorComp) continue;

            URotationMovementComponent* RotationMovementComponent = Cast<URotationMovementComponent>(ActorComp);
            UProjectileMovementComponent* ProjectileMovementComponent = Cast<UProjectileMovementComponent>(ActorComp);

            FComponentData* ComponentData;

            if (RotationMovementComponent)
            {
                ComponentData = new FRotationMovementComponentData;
                RotationMovementComponent->Serialize(ComponentData);
            }
            else if (ProjectileMovementComponent)
            {
                ComponentData = new FProjectileMovementComponentData;
                ProjectileMovementComponent->Serialize(ComponentData);
            }
            else
            {
                continue;
            }

            SceneData.Components.push_back(ComponentData);
        }
    }

    // Scene 디렉터리에 저장
    FSceneLoader::Save(SceneData, SceneName);

    // 동적할당 free
    for (FComponentData* ComponentData : SceneData.Components)
    {
        delete ComponentData;
    }
}

void UWorld::LoadScene(const FString& SceneName)
{
    namespace fs = std::filesystem;
    fs::path path = fs::path("Scene") / SceneName;
    if (path.extension().string() != ".Scene")
    {
        path.replace_extension(".Scene");
    }

    const FString FilePath = path.make_preferred().string();

    // NextUUID 업데이트
    uint32 LoadedNextUUID = 0;
    if (FSceneLoader::TryReadNextUUID(FilePath, LoadedNextUUID))
    {
        if (LoadedNextUUID > UObject::PeekNextUUID())
        {
            UObject::SetNextUUID(LoadedNextUUID);
        }
    }

    // 기존 씬 비우기
    CreateNewScene();

    // V2 데이터 로드
    FSceneData SceneData = FSceneLoader::Load(FilePath);

    // 마우스 델타 초기화
    const FVector2D CurrentMousePos = UInputManager::GetInstance().GetMousePosition();
    UInputManager::GetInstance().SetLastMousePosition(CurrentMousePos);

    if (MainCameraActor && MainCameraActor->GetCameraComponent())
    {
        MainCameraActor->DeSerialize(&SceneData.Camera);
        
        // UIManager의 카메라 회전 상태도 동기화
        UIManager.UpdateMouseRotation(SceneData.Camera.Rotation.Y, SceneData.Camera.Rotation.Z);
        // UI 위젯에 현재 카메라 상태로 재동기화 요청
        UIManager.SyncCameraControlFromCamera();
    }

    // UUID → Object 매핑 테이블
    TMap<uint32, AActor*> ActorMap;
    TMap<uint32, UActorComponent*> ComponentMap;

    // ========================================
    // Pass 1: Actor 및 Component 생성
    // ========================================
    for (const FActorData& ActorData : SceneData.Actors)
    {
        AActor* NewActor = Cast<AActor>(NewObject(ActorData.Type));

        if (!NewActor)
        {
            UE_LOG("Failed to create Actor: %s", ActorData.Type.c_str());
            continue;
        }

        NewActor->DeSerialize(const_cast<FActorData*>(&ActorData));
        NewActor->SetWorld(this);

        ActorMap.Add(ActorData.UUID, NewActor);
    }

    // Component 생성
    for (FComponentData* ComponentData : SceneData.Components)
    {
        UActorComponent* NewComponent = Cast<UActorComponent>(NewObject(ComponentData->Type));

        if (!NewComponent)
        {
            UE_LOG("Failed to create Component: %s", ComponentData->Type.c_str());
            continue;
        }

        NewComponent->DeSerialize(ComponentData);

        // Owner Actor 설정
        if (AActor** OwnerActor = ActorMap.Find(ComponentData->OwnerActorUUID))
        {
            NewComponent->SetOwner(*OwnerActor);
        }

        ComponentMap.Add(ComponentData->UUID, NewComponent);
    }

    // ========================================
    // Pass 2: Actor-Component 연결 및 계층 구조 설정
    // ========================================
    for (const FActorData& ActorData : SceneData.Actors)
    {
        AActor** ActorPtr = ActorMap.Find(ActorData.UUID);
        if (!ActorPtr || !*ActorPtr) continue;

        AActor* Actor = *ActorPtr;

        UActorComponent** ActorComponent = ComponentMap.Find(ActorData.RootComponentUUID);
        // 비계층 컴포넌트는 RootComponentUUID가 없음
        if (!ActorComponent || !*ActorComponent)
            continue;

        // RootComponent 설정
        if (USceneComponent* RootCompPtr = Cast<USceneComponent>(*ActorComponent))
        {
            Actor->RootComponent = RootCompPtr;
        }
    }

    //// Component 부모-자식 관계 설정
    //for (FComponentData* ComponentData : SceneData.Components)
    //{
    //    AActor** OwnerActorPtr = ActorMap.Find(ComponentData->OwnerActorUUID);
    //    if (!OwnerActorPtr) continue;
    //    AActor* OwnerActor = *OwnerActorPtr;
    //    if (!OwnerActor) continue;

    //    UActorComponent** ComponentPtr = ComponentMap.Find(ComponentData->UUID);
    //    if (!ComponentPtr) continue;

    //    UActorComponent* Component = *ComponentPtr;
    //    if (!Component) continue;

    //    // 계층 컴포넌트인 경우
    //    if (ComponentData->IsHierarchical)
    //    {
    //        FSceneComponentData* SceneComponentData = dynamic_cast<FSceneComponentData*>(ComponentData);

    //        USceneComponent* SceneComponent = Cast<USceneComponent>(Component);

    //        // 부모 컴포넌트 연결 (ParentUUID가 0이 아니면)
    //        if (SceneComponentData->ParentComponentUUID != 0)
    //        {
    //            UActorComponent** ParentComponentPtr = ComponentMap.Find(SceneComponentData->ParentComponentUUID);
    //            if (!ParentComponentPtr) continue;

    //            if (USceneComponent* ParentPtr = Cast<USceneComponent>(*ParentComponentPtr))
    //            {
    //                SceneComponent->SetupAttachment(ParentPtr, EAttachmentRule::KeepRelative);
    //            }
    //        }

    //        // Actor의 OwnedComponents에 추가
    //        OwnerActor->OwnedSceneComponents.Add(SceneComponent);
    //    }
    //    // 비계층 컴포넌트인 경우
    //    else
    //    {
    //        OwnerActor->OwnedNonSceneComponents.Add(Component);
    //    }
    //}

    // 계층 컴포넌트 부모-자식 관계 설정
    for (FComponentData* ComponentData : SceneData.Components)
    {
        if (!ComponentData->IsHierarchical) continue;  // 계층만 처리

        FSceneComponentData* SceneComponentData = dynamic_cast<FSceneComponentData*>(ComponentData);
        if (!SceneComponentData) continue;  // SceneComponentData 타입만

        AActor** OwnerActorPtr = ActorMap.Find(SceneComponentData->OwnerActorUUID);
        if (!OwnerActorPtr || !*OwnerActorPtr) continue;

        UActorComponent** ComponentPtr = ComponentMap.Find(SceneComponentData->UUID);
        if (!ComponentPtr || !*ComponentPtr) continue;

        USceneComponent* SceneComponent = Cast<USceneComponent>(*ComponentPtr);
        if (!SceneComponent) continue;  // SceneComponent 타입만

        // 부모 컴포넌트 연결
        if (SceneComponentData->ParentComponentUUID != 0)
        {
            UActorComponent** ParentComponentPtr = ComponentMap.Find(SceneComponentData->ParentComponentUUID);
            if (ParentComponentPtr && *ParentComponentPtr)
            {
                if (USceneComponent* ParentComp = Cast<USceneComponent>(*ParentComponentPtr))
                {
                    SceneComponent->SetupAttachment(ParentComp, EAttachmentRule::KeepRelative);
                }
            }
        }

        // Actor의 OwnedSceneComponents에 추가
        (*OwnerActorPtr)->OwnedSceneComponents.Add(SceneComponent);
    }

    // 비계층 컴포넌트 설정
    for (FComponentData* ComponentData : SceneData.Components)
    {
        if (ComponentData->IsHierarchical) continue;  // 비계층만 처리

        AActor** OwnerActorPtr = ActorMap.Find(ComponentData->OwnerActorUUID);
        if (!OwnerActorPtr || !*OwnerActorPtr) continue;

        UActorComponent** ComponentPtr = ComponentMap.Find(ComponentData->UUID);
        if (!ComponentPtr || !*ComponentPtr) continue;

        UActorComponent* Component = *ComponentPtr;

        // Actor의 OwnedNonSceneComponents에 추가
        (*OwnerActorPtr)->OwnedNonSceneComponents.Add(Component);
    }

    // Actor를 Level에 추가
    for (auto& Pair : ActorMap)
    {
        AActor* Actor = Pair.second;
        Level->AddActor(Actor);

        // StaticMeshActor 전용 포인터 재설정
        if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor))
        {
            StaticMeshActor->SetStaticMeshComponent( Cast<UStaticMeshComponent>(StaticMeshActor->RootComponent));

            // CollisionComponent 찾기
            for (UActorComponent* Comp : StaticMeshActor->OwnedSceneComponents)
            {
                if (UAABoundingBoxComponent* BBoxComp = Cast<UAABoundingBoxComponent>(Comp))
                {
                    StaticMeshActor->CollisionComponent = BBoxComp;
                    StaticMeshActor->SetCollisionComponent(EPrimitiveType::Sphere);
                    break;
                }
            }
        }
    }

    for (FComponentData* Data : SceneData.Components)
    {
        delete Data;
    }

    DeSerialize(&SceneData);

    if (Level)
    {
        InitializeSceneGraph(Level->GetActors());
    }

    UE_LOG("Scene loaded successfully: %s", SceneName.c_str());
}

AGizmoActor* UWorld::GetGizmoActor()
{
    return GizmoActor;
}

UWorld* UWorld::DuplicateWorldForPIE(UWorld* EditorWorld)
{
    if (!EditorWorld)
    {
        return nullptr;
    }

    // 새로운 PIE 월드 생성
    UWorld* PIEWorld = NewObject<UWorld>();
    if (!PIEWorld)
    {
        return nullptr;
    }

    PIEWorld->Renderer = EditorWorld->Renderer;
    PIEWorld->MainViewport = EditorWorld->MainViewport;
    PIEWorld->MultiViewport = EditorWorld->MultiViewport;

    // WorldType을 PIE로 설정
    PIEWorld->WorldType=(EWorldType::PIE);

    //// Renderer 공유 (얕은 복사)
    //PIEWorld->Renderer = EditorWorld->Renderer;

    // MainCameraActor 공유 (PIE는 일단 Editor 카메라 사용)
    PIEWorld->MainCameraActor = EditorWorld->MainCameraActor;

    // GizmoActor는 PIE에서 사용하지 않음
    PIEWorld->GizmoActor = nullptr;

    // GridActor 공유 (선택적)
    PIEWorld->GridActor = nullptr;

    // Level 복제
    if (EditorWorld->GetLevel())
    {
        ULevel* EditorLevel = EditorWorld->GetLevel();

        // Level의 Actors를 복제
        for (AActor* EditorActor : EditorLevel->GetActors())
        {
            if (EditorActor)
            {
                AActor* PIEActor = Cast<AActor>(EditorActor->Duplicate());//체크!

                if (PIEActor)
                {
                    PIEWorld->SpawnActor(PIEActor);
                }
            }
        }
    }
    // PIE 월드 레벨에 복제된 액터들로 BVH 재빌드
    if (PIEWorld->Level)
    {
        PIEWorld->InitializeSceneGraph(PIEWorld->Level->GetActors());
    }
    return PIEWorld;
}

void UWorld::InitializeActorsForPlay()
{
    // 모든 액터의 BeginPlay 호출
    if (Level)
    {
        for (AActor* Actor : Level->GetActors())
        {
            if (Actor)
            {
                Actor->BeginPlay();
            }
        }
    }
}

void UWorld::CleanupWorld()
{
    if (Level)
    {
        for (AActor* Actor : Level->GetActors())
        {
            if (Actor)
            {
                Actor->EndPlay(EEndPlayReason::Quit);
            }
        }
    }
}

void UWorld::Serialize(FObjectData* Data)
{
    FSceneData* SceneData = dynamic_cast<FSceneData*>(Data);
    assert(SceneData, "UStaticMeshComponent::Serialize got wrong data type.");

    SceneData->Version = 2;
    SceneData->NextUUID = UObject::PeekNextUUID();
}

void UWorld::DeSerialize(FObjectData* Data)
{
    FSceneData* SceneData = dynamic_cast<FSceneData*>(Data);
    assert(SceneData, "UStaticMeshComponent::DeSerialize got wrong data type.");

    // NextUUID 업데이트 (로드된 모든 UUID + 1)
    uint32 MaxUUID = SceneData->NextUUID;
    if (MaxUUID > UObject::PeekNextUUID())
    {
        UObject::SetNextUUID(MaxUUID);
    }
}
/**
* @brief Level의 Actor들이 가진 모든 PrimitiveComponent를 가지고 BVH에 빌드 요청
*/
void UWorld::RequestRebuildBVH()
{
    if (BVH)
    {
        TArray<UPrimitiveComponent*> Primitives;
        for (AActor* Actor : Level->GetActors())
        {
            if (Actor)
            {
                for (UActorComponent* Component : Actor->GetComponents())
                {
                    if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
                    {
                        Primitives.Add(Primitive);
                    }
                }
            }
        }
        BVH->Build(Primitives);
    }
}

FFXAABufferType& UWorld::GetFXAAParameters(bool& OutFlag)
{
    OutFlag = bIsFXAAEnabled;
    return FXAAData;
}

/**
 * @brief 이미 생성한 Actor를 spawn하기 위한 shortcut 함수
 * @param InActor World에 생성할 Actor
 */
void UWorld::SpawnActor(AActor* InActor)
{
    InActor->SetWorld(this);

    // 모든 컴포넌트 초기화
    for (UActorComponent* Component : InActor->GetComponents())
    {
        if (Component)
        {
            Component->InitializeComponent();
        }
    }

    if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(InActor->RootComponent))
    {
        FString ActorName = GenerateUniqueActorName(
            GetBaseNameNoExt(StaticMeshComponent->GetStaticMesh()->GetAssetPathFileName())
        );
        InActor->SetName(ActorName);
    }
    else
    {
        FString ActorName = GenerateUniqueActorName(
            InActor->GetClass()->Name
        );
        InActor->SetName(ActorName);
    }

    Level->GetActors().Add(InActor);

    // BVH가 없으면 새로 생성
    if (!BVH)
    {
        BVH = new FBVH();
    }
    // 있는 경우 재빌드
    RequestRebuildBVH();
    
}
