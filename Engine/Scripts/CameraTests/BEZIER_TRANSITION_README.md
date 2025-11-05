# 베지어 카메라 트랜지션 가이드

## 개요

언리얼 엔진 스타일의 카메라 트랜지션 시스템이 이제 **커스텀 베지어 곡선**을 지원합니다!

베지어 곡선을 사용하면 CSS `transition-timing-function`이나 애니메이션 툴에서 사용하는 것과 동일한 방식으로 매우 정밀한 카메라 움직임 제어가 가능합니다.

---

## 베지어 곡선이란?

베지어 곡선은 4개의 포인트를 사용하여 시간에 따른 움직임의 가속도를 정의합니다:

```
P0 (0, 0) -------- P1 (x1, y1)

P3 (1, 1) -------- P2 (x2, y2)
```

- **P0 (0, 0)**: 시작점 (고정)
- **P1 (x1, y1)**: 첫 번째 컨트롤 포인트 (조절 가능)
- **P2 (x2, y2)**: 두 번째 컨트롤 포인트 (조절 가능)
- **P3 (1, 1)**: 끝점 (고정)

컨트롤 포인트 `(x1, y1, x2, y2)`를 조절하여 원하는 타이밍 커브를 만듭니다.

---

## 사용법

### 1. C++에서 사용하기

```cpp
#include "Manager/Camera/Public/PlayerCameraManager.h"
#include "Global/Function.h"  // ECameraEaseType

APlayerCameraManager* CamMgr = World->GetPlayerCameraManager();

// 베지어 컨트롤 포인트 설정 (easeOutQuad)
float BezierCP[4] = { 0.25f, 0.46f, 0.45f, 0.94f };

// 베지어를 사용한 카메라 트랜지션
CamMgr->StartTransitionToLocation(
    FVector(500, 0, 200),      // 목표 위치
    FRotator(-20, 180, 0),     // 목표 회전
    2.0f,                      // 지속 시간
    ECameraEaseType::Bezier,   // 베지어 모드
    -1.0f,                     // FOV (기본값 유지)
    BezierCP                   // 베지어 컨트롤 포인트
);
```

또는 미리 설정:

```cpp
// 베지어 컨트롤 포인트를 미리 설정
CamMgr->SetTransitionBezierControlPoints(BezierCP);

// 이후 StartTransitionToLocation 호출 시 자동으로 사용됨
CamMgr->StartTransitionToLocation(
    FVector(500, 0, 200),
    FRotator(-20, 180, 0),
    2.0f,
    ECameraEaseType::Bezier  // BezierCP 없이 호출해도 이전에 설정한 값 사용
);
```

### 2. Lua 스크립트에서 사용하기

```lua
local camMgr = world:GetPlayerCameraManager()

-- 베지어 컨트롤 포인트 설정 (easeOutCubic)
camMgr:SetTransitionBezierControlPoints(0.215, 0.61, 0.355, 1.0)

-- 베지어를 사용한 카메라 트랜지션
camMgr:StartTransitionToLocation(
    FVector(500, 0, 200),      -- 목표 위치
    FRotator(-20, 180, 0),     -- 목표 회전
    2.0,                       -- 지속 시간
    ECameraEaseType.Bezier     -- 베지어 모드
)
```

---

## 베지어 프리셋

다양한 애니메이션 효과를 위한 베지어 프리셋:

| 이름 | 컨트롤 포인트 | 설명 |
|------|--------------|------|
| **EaseOutQuad** | `(0.25, 0.46, 0.45, 0.94)` | 빠르게 시작, 천천히 끝 (기본값) |
| **EaseInQuad** | `(0.55, 0.085, 0.68, 0.53)` | 천천히 시작, 빠르게 끝 |
| **EaseInOutQuad** | `(0.455, 0.03, 0.515, 0.955)` | 양쪽 모두 부드럽게 |
| **EaseOutCubic** | `(0.215, 0.61, 0.355, 1.0)` | 더 강한 감속 |
| **EaseInCubic** | `(0.55, 0.055, 0.675, 0.19)` | 더 강한 가속 |
| **EaseInOutCubic** | `(0.645, 0.045, 0.355, 1.0)` | S자 커브 (부드러움) |
| **EaseOutElastic** | `(0.68, -0.55, 0.265, 1.55)` | 탄성 효과 (튕김) |

---

## 테스트 스크립트

### CameraBezierTest.lua

베지어 곡선을 사용한 카메라 트랜지션을 테스트하는 스크립트입니다.

#### 사용 방법:
1. 에디터에서 Actor 생성
2. `UseScript` 체크
3. `ScriptName`을 `CameraBezierTest`로 설정
4. Play 시작

#### 컨트롤:
- **숫자 1-5 키**: 특정 뷰로 전환
- **SPACE**: 모든 뷰를 순환
- **B**: 베지어 프리셋 변경 (7가지 프리셋 순환)
- **S**: 트랜지션 중단
- **Q/E**: 트랜지션 속도 조절 (느리게/빠르게)
- **R**: 첫 번째 뷰로 리셋

#### 기능:
- 7가지 베지어 프리셋 실시간 전환
- 5가지 카메라 뷰포인트
- 실시간 진행도 표시
- 각 트랜지션마다 베지어 컨트롤 포인트 로그 출력

---

## 고급 사용법

### 1. 커스텀 베지어 곡선 만들기

웹 도구를 사용하여 커스텀 베지어 곡선을 만들 수 있습니다:
- **cubic-bezier.com** - 실시간 프리뷰와 프리셋 제공
- **easings.net** - 다양한 프리셋 모음

### 2. 베지어를 다른 Easing과 비교

```cpp
// 기본 EaseInOut
CamMgr->StartTransitionToLocation(Loc, Rot, 2.0f, ECameraEaseType::EaseInOut);

// 베지어로 동일한 효과 재현
float BezierCP[4] = { 0.42f, 0.0f, 0.58f, 1.0f };  // EaseInOut과 비슷
CamMgr->StartTransitionToLocation(Loc, Rot, 2.0f, ECameraEaseType::Bezier, -1.0f, BezierCP);
```

### 3. 시네마틱 카메라 워크

```lua
-- 극적인 줌인 (빠르게 시작, 천천히 끝)
camMgr:SetTransitionBezierControlPoints(0.1, 0.7, 0.3, 1.0)
camMgr:StartTransitionToLocation(CloseUpLoc, CloseUpRot, 3.0, ECameraEaseType.Bezier)

-- 부드러운 페이드 아웃 (매우 느린 감속)
camMgr:SetTransitionBezierControlPoints(0.0, 0.0, 0.2, 1.0)
camMgr:StartTransitionToLocation(FarAwayLoc, FarAwayRot, 5.0, ECameraEaseType.Bezier)
```

### 4. 액터 추적 + 베지어

```cpp
// 베지어 컨트롤 포인트 설정
float BezierCP[4] = { 0.68f, -0.55f, 0.265f, 1.55f };  // Elastic

// 액터를 추적하면서 탄성 효과 적용
CamMgr->StartTransitionToActor(
    TargetActor,
    2.0f,
    ECameraEaseType::Bezier,
    FVector(-200, 0, 100),  // 오프셋
    BezierCP
);
```

---

## 기술적 세부사항

### 구현 방식

베지어 곡선 평가는 **Newton-Raphson 방법**을 사용하여 정확도와 성능을 보장합니다:

```cpp
// Function.h에서 구현
inline float EvaluateBezier(float x, const float P[4])
{
    // Newton-Raphson으로 t 값 찾기
    // 최대 8번 반복, 0.001 정밀도
    ...
}
```

### 성능

- **계산 비용**: 프레임당 1회 (트랜지션 중일 때만)
- **반복 횟수**: 최대 8회 (보통 3-4회면 수렴)
- **정밀도**: 0.001 (충분히 부드러운 애니메이션)

### 기본값

베지어 모드를 사용하되 컨트롤 포인트를 지정하지 않으면 **easeOutQuad** 프리셋이 사용됩니다:

```cpp
// 기본 베지어 (easeOutQuad)
CamMgr->StartTransitionToLocation(Loc, Rot, 2.0f, ECameraEaseType::Bezier);
```

---

## FAQ

### Q: 베지어와 다른 Easing 타입의 차이는?

**A:** 다른 Easing 타입들은 고정된 수학 함수입니다:
- `Linear`: `alpha`
- `EaseIn`: `alpha²`
- `EaseOut`: `alpha * (2 - alpha)`
- 등등...

**베지어**는 컨트롤 포인트를 자유롭게 조절하여 원하는 커브를 만들 수 있습니다.

### Q: 컨트롤 포인트 값의 범위는?

**A:**
- **X 값 (x1, x2)**: 반드시 `[0, 1]` 범위 (시간)
- **Y 값 (y1, y2)**: 일반적으로 `[0, 1]`이지만, 튕김 효과를 위해 범위를 벗어날 수 있음 (예: `-0.55` 또는 `1.55`)

### Q: 베지어가 잘 작동하지 않는다면?

**A:**
1. **컨트롤 포인트 확인**: X 값이 `[0, 1]` 범위인지 확인
2. **Easing 타입 확인**: `ECameraEaseType::Bezier`로 설정했는지 확인
3. **로그 확인**: 베지어 CP가 제대로 설정되었는지 확인

```lua
-- 디버그 출력
Print("Bezier CP: " .. preset.cp[1] .. ", " .. preset.cp[2] .. ", " .. preset.cp[3] .. ", " .. preset.cp[4])
```

---

## 예제 시나리오

### 시나리오 1: 보스 등장 씬

```lua
-- 1단계: 빠르게 보스에게 줌인 (1초)
camMgr:SetTransitionBezierControlPoints(0.1, 0.7, 0.3, 1.0)
camMgr:StartTransitionToLocation(BossCloseUp, BossRotation, 1.0, ECameraEaseType.Bezier)

-- 대기...

-- 2단계: 천천히 뒤로 물러나며 전체 뷰 (3초)
camMgr:SetTransitionBezierControlPoints(0.0, 0.0, 0.2, 1.0)
camMgr:StartTransitionToLocation(BattleView, BattleRotation, 3.0, ECameraEaseType.Bezier)
```

### 시나리오 2: 충격 효과

```lua
-- 탄성 베지어로 튕기는 효과
camMgr:SetTransitionBezierControlPoints(0.68, -0.55, 0.265, 1.55)
camMgr:StartTransitionToLocation(ImpactView, ImpactRotation, 0.8, ECameraEaseType.Bezier)
```

### 시나리오 3: 부드러운 컷신 시퀀스

```lua
local cinematicShots = {
    { loc = Shot1Loc, rot = Shot1Rot, bezier = {0.215, 0.61, 0.355, 1.0}, duration = 4.0 },
    { loc = Shot2Loc, rot = Shot2Rot, bezier = {0.645, 0.045, 0.355, 1.0}, duration = 3.5 },
    { loc = Shot3Loc, rot = Shot3Rot, bezier = {0.25, 0.46, 0.45, 0.94}, duration = 5.0 }
}

for i, shot in ipairs(cinematicShots) do
    camMgr:SetTransitionBezierControlPoints(shot.bezier[1], shot.bezier[2], shot.bezier[3], shot.bezier[4])
    camMgr:StartTransitionToLocation(shot.loc, shot.rot, shot.duration, ECameraEaseType.Bezier)

    -- Wait for transition to complete...
end
```

---

## 참고 자료

- **cubic-bezier.com** - 인터랙티브 베지어 에디터
- **easings.net** - 다양한 Easing 함수 프리셋
- **CSS Cubic Bezier** - 웹 표준과 동일한 방식

---

**즐거운 카메라 워크 되세요!** 🎥✨
