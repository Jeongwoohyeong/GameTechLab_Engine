# FutureEngine - Game Jam Week 9

## 프로젝트 개요

**제작 기간**: 2025.11.01 ~ 2025.11.07
**주간 주제**: Game Jam #3 - Make sure to delegate (확실히 위임하라) + Light, Camera, Action!

이번 프로젝트는 **LuaJIT 스크립트 시스템**과 **시네마틱 카메라 연출**을 통합하여 완성도 높은 3D 슈팅 게임을 제작하는 것을 목표로 합니다. 지금까지 제작된 컴포넌트들을 활용하고, Lua 스크립트로 게임 로직을 구현하며, 다양한 카메라 효과와 타격감 향상 기법을 적용했습니다.

---

## 게임 소개

### 장르
3D Top-Down Shooter (탑다운 슈팅 게임)

### 게임 플레이
- **플레이어**: WASD로 이동, 마우스 좌클릭으로 미사일 발사
- **목표**: 웨이브로 등장하는 적을 처치하여 최대한 높은 점수 획득
- **적 AI**: 플레이어를 향해 추적하며 충돌 시 데미지
- **HP 시스템**: 플레이어 HP가 0이 되면 게임 오버
- **스코어**: 적 처치 시 +100점, 실시간 HUD에 표시

### 게임 흐름
1. **메인 메뉴**: Start Game 버튼 클릭 시 게임 시작
2. **시네마틱 인트로**: 레터박스 효과와 함께 게임 시작 연출
3. **인플레이**: 적 웨이브 스폰, 플레이어 전투
4. **게임 오버**: 사망 시 게임 오버 화면, Restart 버튼으로 재시작

---

## 학습 목표 및 구현 사항

### Game Jam #3 - Make sure to delegate

#### 1. LuaJIT 스크립트 시스템 (Sol2 통합)

**구현 위치**: `Engine/Source/Manager/Lua/`

**핵심 기능**:
- **Sol2 Wrapper**: C++ <-> Lua 바인딩을 위한 Sol2 라이브러리 통합
- **Template 스크립트**: `Engine/Scripts/template.lua` - Factory 패턴으로 Actor별 독립된 인스턴스 생성
- **Hot Reload**: 스크립트 편집 후 즉시 반영 (실시간 디버깅 가능)
- **엔진 타입 바인딩**: `FVector`, `FRotator`, `FVector4` 등 엔진 타입을 Lua에서 사용 가능

**주요 스크립트**:
```
Engine/Scripts/
├── template.lua                    # 기본 템플릿 (Factory 패턴)
├── Player/PlayerCharacter.lua      # 플레이어 로직 (무기, 카메라)
├── Enemy/Enemy.lua                 # 적 AI 및 HP 시스템
├── Missile/Missile.lua             # 미사일 발사체 로직
├── GameMode/InPlayGameMode.lua     # 게임 모드 (웨이브 스포너)
└── GameMode/MainMenuGameMode.lua   # 메인 메뉴
```

**스크립트 생성 워크플로우**:
1. 에디터에서 Actor 배치
2. "Create Script" 버튼 클릭 → `SceneName_ActorName_UUID.lua` 자동 생성
3. "Edit Script" 버튼 클릭 → 연결된 텍스트 에디터로 스크립트 편집
4. BeginPlay, Tick, OnOverlap, OnHit 등 이벤트 함수 구현

**구현 파일**:
- `Engine/Source/Manager/Lua/Public/LuaScriptManager.h`
- `Engine/Source/Manager/Lua/Private/LuaScriptManager.cpp`

---

#### 2. Delegate 시스템 (가변 인자 템플릿)

**구현 위치**: `Engine/Source/Global/Delegate.h`

**핵심 기능**:
- **TDelegate<Args...>**: 가변 인자 템플릿 기반 델리게이트
- **AddDynamic**: 멤버 함수 바인딩
  ```cpp
  OnComponentHit.AddDynamic(this, &APlayerCharacter::HandleHit);
  ```
- **Broadcast**: 등록된 모든 핸들러 일괄 실행
  ```cpp
  OnComponentHit.Broadcast(HitComp, OtherActor, ...);
  ```
- **WeakObjectPtr 안전성**: 삭제된 객체 자동 무시

**사용 예시** (충돌 이벤트):
```cpp
// PrimitiveComponent.h
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComponentHit,
    UPrimitiveComponent*, AActor*, UPrimitiveComponent*, FVector, const FHitResult&);

FOnComponentHit OnComponentHit;

// BoxComponent에서 충돌 시
OnComponentHit.Broadcast(this, OtherActor, OtherComp, ImpactNormal, Hit);
```

**구현 파일**:
- `Engine/Source/Global/Delegate.h` (템플릿 헤더)
- `Engine/Source/Global/DelegateMacros.h` (매크로 정의)

---

#### 3. Coroutine 시스템

**구현 위치**: `Engine/Source/Manager/Coroutine/Public/LuaCoroutineManager.h`

**핵심 기능**:
- **비동기 작업**: `wait(seconds)`, `wait_until(condition)`
- **상태 관리**: Running, WaitingForTime, WaitingForCondition, Dead
- **멀티 코루틴**: 각 Actor가 독립적으로 여러 코루틴 실행 가능
- **실시간 Tick**: TimeManager의 DeltaTime으로 타이머 갱신

**Lua 사용 예시** (게임 모드 웨이브 스포너):
```lua
-- InPlayGameMode.lua
function ReturnTable:WaveSpawner()
    local TotalSpawned = 0
    while TotalSpawned < self.MaxPoolSize do
        local SpawnCount = math.min(self.CurrentWave * 2, self.MaxPoolSize - TotalSpawned)

        Print(string.format("[GameMode] Wave %d spawned %d enemies", self.CurrentWave, SpawnCount))

        for i = 1, SpawnCount do
            local RandomPos = FVector(
                (math.random() - 0.5) * 1000.0,
                (math.random() - 0.5) * 1000.0,
                (math.random() - 0.5) * 1000.0
            )
            self.GameMode:SpawnEnemies(1, RandomPos)
        end

        TotalSpawned = TotalSpawned + SpawnCount
        self.CurrentWave = self.CurrentWave + 1

        wait(5.0)  -- 5초 대기 후 다음 웨이브
    end
end

-- 코루틴 시작
StartCoroutine(self, "WaveSpawner")
```

**구현 파일**:
- `Engine/Source/Manager/Coroutine/Public/LuaCoroutineManager.h`
- `Engine/Source/Manager/Coroutine/Private/LuaCoroutineManager.cpp`

---

#### 4. Component Hit/Overlap Event

**구현 위치**: `Engine/Source/Component/`

**컴포넌트 타입**:
- **UBoxComponent**: 박스 형태 충돌체 (`BoxExtent` 설정)
- **USphereComponent**: 구 형태 충돌체 (`SphereRadius` 설정)
- **UCapsuleComponent**: 캡슐 형태 충돌체 (`CapsuleHalfHeight`, `CapsuleRadius`)

**이벤트 종류**:
1. **OnComponentBeginOverlap**: 오버랩 시작 (겹침 감지, 물리 차단 X)
2. **OnComponentEndOverlap**: 오버랩 종료
3. **OnComponentHit**: 충돌 (물리 차단 O)

**충돌 설정**:
```cpp
// PrimitiveComponent
bool bGenerateOverlapEvents = true;  // 오버랩 이벤트 활성화
bool bGenerateHitEvents = false;     // Hit 이벤트 활성화
bool bBlockComponent = true;         // 물리 충돌 차단
```

**Lua에서 사용** (적 캐릭터 데미지 처리):
```lua
-- Enemy.lua
function ReturnTable:OnBeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult)
    if bIsDead then
        return  -- 죽은 상태면 무시
    end

    local playerChar = Cast_APlayerCharacter(OtherActor)
    if playerChar then
        -- 플레이어에게 데미지 (HUD 업데이트)
        local hudWidget = GetGameUIManager():GetHUDWidget()
        hudWidget:TakeDamage(15)
    end
end
```

**구현 파일**:
- `Engine/Source/Component/Public/PrimitiveComponent.h`
- `Engine/Source/Component/Collision/Public/BoxComponent.h`
- `Engine/Source/Component/Collision/Public/SphereComponent.h`
- `Engine/Source/Component/Collision/Public/CapsuleComponent.h`

---

#### 5. GameMode 시스템 (시작/끝/재시작)

**구현 위치**: `Engine/Source/GameMode/Public/GameMode.h`

**게임 상태**:
```cpp
enum class EGameState : uint8 {
    MainMenu,   // 메인 메뉴
    InPlay,     // 인플레이
    GameOver    // 게임 오버
};
```

**핵심 메서드**:
```cpp
void ChangeState(EGameState NewState);              // 상태 전환
void SpawnPlayerCharacter();                        // 플레이어 스폰
void InitializeEnemyPool(int32 EnemyCount);         // 적 풀 초기화
void SpawnEnemies(int32 Count, FVector Location);   // 적 스폰
void AddScore(int32 ScoreToAdd);                    // 스코어 추가
int32 GetCurrentScore() const;                      // 현재 스코어 반환
```

**Lua GameMode 스크립트** (`InPlayGameMode.lua`):
```lua
function ReturnTable:StartGame()
    Print("[GameMode] START GAME!")

    -- 레터박스 활성화 (시네마틱 시작)
    local camMgr = PlayerController:GetPlayerCameraManager()
    camMgr:StartLetterBox(0.1, 0.5)  -- 높이 10%, 블렌드 0.5초

    -- 5초 후 레터박스 페이드아웃
    StartCoroutine(self, "LetterboxFadeOut")

    -- 적 풀 초기화 (30마리)
    self.GameMode:InitializeEnemyPool(30)

    -- 웨이브 스포너 시작
    StartCoroutine(self, "WaveSpawner")

    -- 플레이어 무기 활성화
    PlayerPawn:GetLuaScriptComponent():ActivateFunction("EnableWeapon")
end
```

**구현 파일**:
- `Engine/Source/GameMode/Public/GameModeBase.h`
- `Engine/Source/GameMode/Public/GameMode.h`
- `Engine/Source/GameMode/Private/GameMode.cpp`
- `Engine/Scripts/GameMode/InPlayGameMode.lua`

---

### Light, Camera, Action!

#### 1. PlayerCameraManager 시스템

**구현 위치**: `Engine/Source/Manager/Camera/Public/PlayerCameraManager.h`

**핵심 기능**:
- **ViewTarget 관리**: 카메라가 따라가는 대상 설정
- **Modifier 시스템**: 여러 카메라 효과를 스택으로 관리
- **Spring Arm**: 플레이어를 따라가는 부드러운 카메라
- **FOV 제어**: Field of View 동적 변경

**초기화**:
```cpp
// 에디터 모드
void Initialize(UCamera* InCamera);

// PIE(Play In Editor) 모드
void Initialize(UCameraComponent* InCameraComponent);
```

**ViewType**:
```cpp
enum class ECameraViewType : uint8 {
    ThirdPerson,    // 3인칭
    FirstPerson,    // 1인칭
    Cinematic,      // 시네마틱
    DeathCam,       // 사망 카메라
    FreeCam         // 자유 카메라
};
```

**Lua에서 카메라 접근**:
```lua
local gameMode = GetGameMode()
local playerController = gameMode:GetPlayerController()
local camMgr = playerController:GetPlayerCameraManager()
```

**구현 파일**:
- `Engine/Source/Manager/Camera/Public/PlayerCameraManager.h`
- `Engine/Source/Manager/Camera/Private/PlayerCameraManager.cpp`

---

#### 2. CameraModifier (모디파이어 시스템)

**구현 위치**: `Engine/Source/Manager/Camera/Public/CameraModifier.h`

**핵심 개념**:
- **알파 블렌딩**: 효과의 강도를 0~1 사이로 부드럽게 전환
- **우선순위**: Priority 값에 따라 여러 모디파이어 순서대로 적용
- **상태 관리**: Disabled, BlendingIn, Active, BlendingOut

**기본 클래스 구조**:
```cpp
class UCameraModifier : public UObject {
public:
    virtual bool ModifyCamera(float DeltaTime, FVector& CameraLocation, FRotator& CameraRotation);
    virtual void UpdateModifier(float DeltaTime);
    virtual void EnableModifier();   // 알파 블렌드 인
    virtual void DisableModifier();  // 알파 블렌드 아웃

protected:
    float AlphaInTime = 0.3f;    // 페이드인 시간
    float AlphaOutTime = 0.3f;   // 페이드아웃 시간
    float Alpha = 0.0f;          // 현재 알파 (0~1)
    uint8 Priority = 128;        // 적용 우선순위
    bool bDisabled = true;       // 비활성화 상태
};
```

**구현 파일**:
- `Engine/Source/Manager/Camera/Public/CameraModifier.h`
- `Engine/Source/Manager/Camera/Private/CameraModifier.cpp`

---

#### 3. Camera Shake (카메라 흔들림)

**구현 위치**: `Engine/Source/Manager/Camera/Public/CameraModifier_CameraShake.h`

**핵심 기능**:
- **강도 조절**: Intensity 파라미터로 흔들림 정도 설정
- **지속 시간**: Duration 후 자동으로 감쇠
- **Bezier Decay**: easeOutQuad 커브로 자연스러운 감쇠
- **위치/회전 흔들림**: Location과 Rotation 모두 랜덤 오프셋 적용

**사용 예시** (플레이어 피격 시):
```lua
-- PlayerCharacter.lua - OnHit 이벤트
function ReturnTable:OnHit(OverlappedComp, OtherActor, OtherComp, NormalImpulse, OutHit)
    local gameMode = GetGameMode()
    local playerController = gameMode:GetPlayerController()
    local camMgr = playerController:GetPlayerCameraManager()

    -- 강도 5.0, 지속시간 1.0초
    camMgr:StartCameraShake(5.0, 1.0)
end
```

**Bezier Decay 커브**:
```cpp
// easeOutQuad (빠르게 시작, 천천히 끝)
float BezierCP[4] = { 0.250f, 0.460f, 0.450f, 0.940f };
```

**구현 파일**:
- `Engine/Source/Manager/Camera/Public/CameraModifier_CameraShake.h`
- `Engine/Source/Manager/Camera/Private/CameraModifier_CameraShake.cpp`

---

#### 4. Letter Box (시네마틱 검은 바)

**구현 위치**: `Engine/Source/Render/RenderPass/Public/LetterboxPass.h`

**핵심 기능**:
- **종횡비 설정**: 21:9, 2.39:1 등 시네마틱 비율
- **알파 블렌딩**: 부드러운 페이드인/아웃
- **Viewport UV 매핑**: 상하단 검은 바 렌더링
- **풀스크린 쿼드**: 전체 화면 오버레이

**사용 예시** (게임 시작 시):
```lua
-- InPlayGameMode.lua
function ReturnTable:StartGame()
    local camMgr = PlayerController:GetPlayerCameraManager()

    -- 레터박스 켜기 (높이 10%, 블렌드 시간 0.5초)
    camMgr:StartLetterBox(0.1, 0.5)

    -- 5초 후 코루틴으로 페이드아웃
    StartCoroutine(self, "LetterboxFadeOut")
end

function ReturnTable:LetterboxFadeOut()
    wait(5.0)

    local camMgr = PlayerController:GetPlayerCameraManager()
    camMgr:StopLetterBox(2.0)  -- 2초에 걸쳐 사라짐
end
```

**상수 버퍼 구조**:
```cpp
struct alignas(16) FLetterboxConstants {
    FVector4 FadeColor;                 // 검은 바 색상
    float FadeAmount;                   // 투명도
    float TargetAspectRatio;            // 목표 종횡비 (21:9 = 2.333)
    float RenderTargetWidth/Height;     // 렌더 타겟 해상도
    float LetterBoxAlpha;               // 알파 블렌드
    float LetterBoxAnimationProgress;   // 애니메이션 진행도
};
```

**구현 파일**:
- `Engine/Source/Render/RenderPass/Public/LetterboxPass.h`
- `Engine/Source/Render/RenderPass/Private/LetterboxPass.cpp`

---

#### 5. Fade In/Out (화면 페이드)

**구현 위치**: `Engine/Source/Manager/Camera/Public/CameraModifier_CameraFadeInOut.h`

**핵심 기능**:
- **Fade Stage**: None → FadeOut → Hold → FadeIn
- **색상 지정**: FVector4(R, G, B, A)로 페이드 색상 설정
- **Hold 시간**: 완전 암전 상태 유지 시간
- **즉시 취소**: `StopFadeInOut(true)` 호출 시 즉시 종료

**사용 예시** (플레이어 스크립트):
```lua
-- PlayerCharacter.lua
function ReturnTable:TriggerCameraFade(FadeOutDuration, FadeInDuration, Color, HoldDuration)
    local camMgr = PlayerController:GetPlayerCameraManager()

    -- FadeOut 1.5초, Hold 0.4초, FadeIn 2.7초, 빨간색
    camMgr:StartFadeInOut(
        1.5,  -- FadeOut 시간
        2.7,  -- FadeIn 시간
        1.0, 0.0, 0.0, 1.0,  -- 빨간색 (R, G, B, A)
        0.4   -- Hold 시간
    )
end

-- F키로 테스트
if InputMgr:IsKeyDown("F") then
    self:TriggerCameraFade(1.5, 2.7, FVector4(1.0, 0.0, 0.0, 1.0), 0.4)
end

-- G키로 즉시 취소
if InputMgr:IsKeyDown("G") then
    camMgr:StopFadeInOut(true)
end
```

**Fade Stage 전환**:
```cpp
enum class EFadeStage : uint8 {
    None,      // 효과 없음
    FadeOut,   // 화면 어두워짐
    Hold,      // 완전 암전 유지
    FadeIn     // 화면 밝아짐
};
```

**구현 파일**:
- `Engine/Source/Manager/Camera/Public/CameraModifier_CameraFadeInOut.h`
- `Engine/Source/Manager/Camera/Private/CameraModifier_CameraFadeInOut.cpp`

---

#### 6. Spring Arm (부드러운 카메라 추적)

**구현 위치**: `Engine/Source/Manager/Camera/Public/PlayerCameraManager.h`

**핵심 기능**:
- **팔 길이**: SpringArmLength로 카메라와 타겟 간 거리 설정
- **오프셋**: SpringArmOffset으로 상대 위치 조정
- **보간 속도**: SpringArmInterpSpeed로 부드러운 이동
- **충돌 감지**: 장애물 감지 시 팔 길이 동적 축소 (옵션)

**설정 예시**:
```cpp
// PlayerCameraManager 내부
bool bSpringArmEnabled = true;
FVector SpringArmOffset = FVector(-300.0f, 0.0f, 100.0f);  // 뒤쪽 300, 위쪽 100
float SpringArmLength = 300.0f;
float SpringArmInterpSpeed = 10.0f;  // 보간 속도
bool bEnableCollisionTest = true;    // 충돌 감지
```

**Lua에서 설정**:
```lua
local camMgr = PlayerController:GetPlayerCameraManager()
camMgr:SetSpringArmParams(
    FVector(-300.0, 0.0, 100.0),  -- 오프셋
    300.0,                        -- 팔 길이
    10.0                          -- 보간 속도
)
camMgr:SetSpringArmEnabled(true)
camMgr:SetSpringArmCollisionEnabled(true)
```

**구현 파일**:
- `Engine/Source/Manager/Camera/Public/PlayerCameraManager.h` (내부 기능)
- `Engine/Source/Manager/Camera/Private/PlayerCameraManager.cpp`

---

#### 7. Camera Transition (카메라 전환)

**구현 위치**: `Engine/Source/Manager/Camera/Public/CameraModifier_CameraTransition.h`

**핵심 기능**:
- **위치 전환**: 특정 위치/회전으로 부드럽게 이동
- **액터 추적**: 특정 액터를 따라가도록 전환
- **Easing Type**: Linear, EaseIn, EaseOut, EaseInOut, Bezier
- **FOV 전환**: 카메라 시야각 동시 변경 가능

**Easing Type**:
```cpp
enum class ECameraEaseType {
    Linear,     // 일정한 속도
    EaseIn,     // 천천히 시작
    EaseOut,    // 천천히 끝
    EaseInOut,  // 양쪽 부드럽게
    Bezier      // 베지어 커브
};
```

**사용 예시 1** - Q키로 탑뷰 전환:
```lua
-- PlayerCharacter.lua
function ReturnTable:TestCameraTransitionToTopView()
    local playerPos = PlayerActor.ActorLocation
    local playerRot = PlayerActor.ActorRotation

    -- 플레이어 머리 위 500 높이
    local localOffset = FVector(-500.0, 0.0, 500.0)
    local worldOffset = playerRot:RotateVector(localOffset)
    local targetPos = playerPos + worldOffset

    -- 플레이어를 바라보는 회전 계산
    local lookDir = (playerPos - targetPos):GetNormalized()
    local yaw = math.atan(lookDir.Y, lookDir.X) * (180.0 / 3.14159265359)
    local pitch = math.asin(-lookDir.Z) * (180.0 / 3.14159265359)
    local targetRot = FRotator(pitch, yaw, 0.0)

    -- 2초 동안 EaseInOut으로 이동
    camMgr:StartTransitionToLocation(
        targetPos,
        targetRot,
        2.0,  -- duration
        3,    -- EaseInOut
        -1    -- FOV 유지
    )
end
```

**사용 예시 2** - 특정 액터 추적:
```lua
function ReturnTable:TransitionCameraToActor(targetActor, duration, offset, easeType)
    local camMgr = PlayerController:GetPlayerCameraManager()

    camMgr:StartTransitionToActor(
        targetActor,
        2.0,                          -- duration
        3,                            -- EaseInOut
        FVector(-300.0, 0.0, 100.0)   -- 오프셋
    )
end
```

**R키로 원래 카메라 복귀**:
```lua
function ReturnTable:TestCameraTransitionReturn()
    local camMgr = PlayerController:GetPlayerCameraManager()
    camMgr:StopCameraTransition()  -- ViewTarget(플레이어)를 다시 따라감
end
```

**구현 파일**:
- `Engine/Source/Manager/Camera/Public/CameraModifier_CameraTransition.h`
- `Engine/Source/Manager/Camera/Private/CameraModifier_CameraTransition.cpp`

---

#### 8. Hit Stop (타격 정지)

**구현 위치**: `Engine/Source/Manager/Time/Public/TimeManager.h`

**핵심 기능**:
- **시간 정지**: 짧은 순간 게임 시간을 0으로 만들어 타격감 강조
- **실시간 기준**: Duration은 실제 시간(Real Time) 기준
- **자동 복구**: Duration 경과 후 자동으로 정상 시간으로 복귀

**사용 예시** (미사일 충돌 시):
```cpp
// Missile.cpp - 적과 충돌 시
void AMissile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, ...)
{
    // 0.2초간 시간 정지 (타격감 강조)
    UTimeManager::GetInstance().StartHitStop(0.2f);

    // 폭발 이펙트, 데미지 처리 등
}
```

**Lua에서 사용**:
```lua
local timeManager = GetTimeManager()
timeManager:StartHitStop(0.2)  -- 0.2초 타격 정지
```

**구현**:
```cpp
void UTimeManager::StartHitStop(float Duration)
{
    HitStopTimeRemaining = Duration;  // 실시간 기준 타이머
    // Tick에서 HitStopTimeRemaining > 0이면 DeltaTime = 0 반환
}

float UTimeManager::GetDeltaTime() const
{
    if (HitStopTimeRemaining > 0.0f)
        return 0.0f;  // 시간 정지

    return DeltaTime * GlobalTimeDilation;
}
```

**구현 파일**:
- `Engine/Source/Manager/Time/Public/TimeManager.h`
- `Engine/Source/Manager/Time/Private/TimeManager.cpp`

---

#### 9. Slow Motion (슬로우 모션)

**구현 위치**: `Engine/Source/Manager/Time/Public/TimeManager.h`

**핵심 기능**:
- **속도 조절**: Speed 파라미터 (0.0 ~ 1.0)
- **지속 시간**: Duration (실시간 기준)
- **자동 복구**: 이전 TimeDilation 값으로 자동 복원
- **Global Time Dilation**: 모든 게임 오브젝트에 일괄 적용

**사용 예시** (적 처치 시):
```cpp
// Enemy.cpp - 죽을 때
void AEnemy::OnDeath()
{
    // 0.3배속으로 3초간 슬로우 모션
    UTimeManager::GetInstance().StartSlowMotion(0.3f, 3.0f);
}
```

**Lua에서 사용**:
```lua
local timeManager = GetTimeManager()

-- 0.3배속으로 3초간 슬로우 모션
timeManager:StartSlowMotion(0.3, 3.0)

-- 즉시 중단
timeManager:StopSlowMotion()

-- 타임 딜레이션 수동 설정
timeManager:SetGlobalTimeDilation(0.5)  -- 0.5배속
timeManager:ResetTimeDilation()         -- 1.0배속으로 복원
```

**구현**:
```cpp
void UTimeManager::StartSlowMotion(float Speed, float Duration)
{
    PreviousTimeDilation = GlobalTimeDilation;  // 이전 값 저장
    SlowMotionSpeed = Speed;
    SlowMotionTimeRemaining = Duration;
    SetGlobalTimeDilation(Speed);
}

void UTimeManager::Update()
{
    // 슬로우 모션 타이머 (실시간 기준)
    if (SlowMotionTimeRemaining > 0.0f)
    {
        SlowMotionTimeRemaining -= RealDeltaTime;

        if (SlowMotionTimeRemaining <= 0.0f)
        {
            // 이전 TimeDilation으로 복원
            SetGlobalTimeDilation(PreviousTimeDilation);
        }
    }
}
```

**구현 파일**:
- `Engine/Source/Manager/Time/Public/TimeManager.h`
- `Engine/Source/Manager/Time/Private/TimeManager.cpp`

---

## 게임 조작법

### 이동 및 전투
- **WASD**: 플레이어 이동
- **마우스 좌클릭**: 미사일 발사 (꾹 누르면 연사)
- **스페이스바**: (미구현 - 향후 점프/대시 등)

### 카메라 테스트 (개발자 모드)
- **Q키**: 플레이어 머리 위에서 내려다보는 카메라로 전환
- **E키**: 플레이어 좌측 측면에서 보는 카메라로 전환
- **R키**: 원래 카메라 위치로 복귀
- **F키**: 화면 페이드 테스트 (빨간색)
- **G키**: 페이드 효과 즉시 취소

---

## 핵심 기술 키워드

### Game Jam #3
- **Lua**: 스크립트 언어
- **LuaJIT**: Just-In-Time 컴파일러
- **Sol2**: C++ Lua 바인딩 라이브러리
- **Template Factory Pattern**: Actor별 독립 인스턴스
- **Hot Reload**: 스크립트 실시간 편집
- **Variadic Argument**: 가변 인자 템플릿
- **TDelegate**: 템플릿 델리게이트
- **Coroutine**: 비동기 작업 (yield, resume, thread)
- **Overlap/Hit Event**: 충돌 감지 이벤트

### Light, Camera, Action!
- **PlayerCameraManager**: 카메라 관리자
- **CameraModifier**: 카메라 효과 모디파이어
- **Alpha Blending**: 알파 블렌드 페이드
- **Camera Shake**: 카메라 흔들림
- **Letter Box**: 시네마틱 검은 바 (21:9 종횡비)
- **Fade In/Out**: 화면 페이드
- **Spring Arm**: 부드러운 카메라 추적
- **Camera Transition**: 카메라 전환 (Easing, Bezier)
- **Hit Stop**: 타격 정지
- **Slow Motion**: 슬로우 모션
- **Global Time Dilation**: 전역 시간 스케일
- **FOV (Field of View)**: 시야각
- **Sense of Hitting**: 타격감 (쉐이크 + 히트스톱)

---

## 프로젝트 구조

```
FutureEngine/
├── Engine/
│   ├── Source/
│   │   ├── Manager/
│   │   │   ├── Lua/                    # Lua 스크립트 매니저
│   │   │   ├── Coroutine/              # 코루틴 매니저
│   │   │   ├── Camera/                 # 카메라 매니저 + 모디파이어
│   │   │   └── Time/                   # 타임 매니저 (HitStop, Slomo)
│   │   ├── Component/
│   │   │   ├── Collision/              # Box, Sphere, Capsule Component
│   │   │   └── Public/                 # PrimitiveComponent (Hit/Overlap)
│   │   ├── GameMode/                   # GameMode, GameState
│   │   ├── Render/RenderPass/          # LetterboxPass
│   │   └── Global/                     # Delegate, DelegateMacros
│   └── Scripts/
│       ├── template.lua                # Factory 패턴 템플릿
│       ├── Player/PlayerCharacter.lua  # 플레이어 로직
│       ├── Enemy/Enemy.lua             # 적 AI
│       ├── Missile/Missile.lua         # 미사일 로직
│       └── GameMode/
│           ├── InPlayGameMode.lua      # 인플레이 모드 (웨이브)
│           ├── MainMenuGameMode.lua    # 메인 메뉴
│           └── GameOverGameMode.lua    # 게임 오버
└── Build/
    └── Release_StandAlone/             # 단독 빌드 제출
```

---

## 학습 자료 및 참고 문서

### Lua & Scripting
- [Lua 공식 사이트](https://www.lua.org/)
- [Sol2 GitHub](https://github.com/ThePhD/sol2)
- [vcpkg (패키지 매니저)](https://github.com/microsoft/vcpkg)
- [Roblox Scripting Docs](https://create.roblox.com/docs/scripting)

### Coroutine
- [Lua Coroutines (PIL)](https://www.lua.org/pil/9.1.html)
- [Unity Coroutines](https://docs.unity3d.com/kr/2022.3/Manual/Coroutines.html)

### Camera System
- [Unreal Engine Camera Shakes](https://dev.epicgames.com/documentation/ko-kr/unreal-engine/camera-shakes-in-unreal-engine)
- [UCameraShakeBase API](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Camera/UCameraShakeBase)
- [FInterpTo API](https://dev.epicgames.com/documentation/en-us/unreal-engine/BlueprintAPI/Math/Interpolation/FInterpTo)
- [UCurveFloat API](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Curves/UCurveFloat)
- [Cinematic Color Fade](https://dev.epicgames.com/documentation/en-us/unreal-engine/cinematic-color-fade-track-in-unreal-engine)

### Game Engine Architecture
- **GEA 16.8**: Scripting System
- **GEA 16.9.3**: Lua Integration
- **GEA 11.4.5**: Camera Systems
- **GEA 11.4.6**: Camera Effects

---

## 빌드 및 실행

### 요구 사항
- Windows 10/11 (64-bit)
- DirectX 11 지원 GPU
- Visual Studio 2022 (C++17)
- vcpkg (Lua, Sol2)

### 빌드 방법
1. `FutureEngine.sln` 열기
2. **Release_StandAlone** 구성 선택
3. F5 또는 Ctrl+F5로 빌드 및 실행

### 실행 파일
- `Build/Release_StandAlone/FutureEngine.exe`

---

## 향후 개선 사항

### 게임 플레이
- [ ] 아이템 시스템 (무기 업그레이드, HP 회복)
- [ ] 보스 전투 (특수 패턴, 시네마틱 연출)
- [ ] 난이도 선택 (Easy, Normal, Hard)
- [ ] 리더보드 (최고 점수 저장)

### 카메라 효과
- [ ] Depth of Field (피사계 심도)
- [ ] Motion Blur (모션 블러)
- [ ] Vignetting (비네팅)
- [ ] Chromatic Aberration (색수차)

### 최적화
- [ ] 오브젝트 풀링 확장 (이펙트, 사운드)
- [ ] LOD (Level of Detail)
- [ ] Occlusion Culling 개선

---

## 제작자

**Team**: GTL Week 09 Game Jam
**개발 기간**: 2025.11.01 ~ 2025.11.07

---

## 라이선스

Educational Project - For Learning Purposes Only
