# 신규 기능 구현 이슈 목록 (프로젝트 맞춤)

현재 코드 베이스(`TL2` 프로젝트) 구조를 반영하여 재구성한 기능 구현 이슈 목록입니다.

---

## ⭐️ 높은 우선순위 ⭐️

### 🌱 Stage 1 — 독립적으로 가능한 초기 이슈들
*(바로 착수 가능, blocked by 없음)*

- **Issue #1: SceneDepth 뷰 모드 시스템 구현**
  - **설명:** `Renderer` 클래스에 뷰 모드 전환을 위한 기본 시스템을 구현합니다. 첫 번째 뷰 모드로, 씬의 깊이 값을 시각화하는 `SceneDepth` 뷰 모드를 추가합니다. 이를 위해 전체 화면을 덮는 쿼드를 렌더링하고 깊이 버퍼를 텍스처로 읽어오는 기능이 필요합니다.
  - **Tags:** `render`, `viewmode`
  - **Blocked by:** -

- **Issue #2: `HeightFogComponent` 정의 (데이터 전용)**
  - **설명:** `ActorComponent`를 상속받는 `HeightFogComponent.h` 및 `.cpp` 파일을 생성합니다. `SpotlightComponent`를 참고하여 Fog의 속성(e.g., `FogDensity`, `FogHeightFalloff`, `FogColor`)을 멤버 변수로 정의하고, 기본값을 설정합니다. 아직 렌더링 로직은 포함하지 않습니다.
  - **Tags:** `engine`, `fog`
  - **Blocked by:** -

- **Issue #3: `FireBallComponent` 정의 (기본 구조)**
  - **설명:** `ActorComponent`를 상속받는 `FireBallComponent.h` 및 `.cpp` 파일을 생성합니다. Post-process 기반 Point Light 효과를 위해 필요한 속성(e.g., `Intensity`, `Radius`, `Color`)의 멤버 변수를 정의합니다.
  - **Tags:** `vfx`, `post-process`
  - **Blocked by:** -

- **Issue #4: FXAA를 위한 Post-Process 기본 패스 구축**
  - **설명:** `Renderer`에 FXAA 적용을 위한 기본적인 Post-Process 패스를 추가합니다. 최종 렌더링된 씬을 텍스처로 받아 전체 화면 쿼드에 다시 렌더링하는 구조를 설정합니다. 실제 FXAA 셰이더 구현은 다음 단계에서 진행합니다.
  - **Tags:** `fxaa`, `render`, `post-process`
  - **Blocked by:** -

### 🧩 Stage 2 — Fog 시스템 기반 구축
*(데이터와 렌더링 로직 연결)*

- **Issue #5: Fog 상수 버퍼(Constant Buffer) 구현**
  - **설명:** Fog 렌더링에 필요한 데이터를 GPU로 전달하기 위한 상수 버퍼(`FogConstants`)를 `D3D11RHI` 또는 관련 셰이더 코드에 정의합니다. `HeightFogComponent`의 데이터를 이 상수 버퍼 구조체로 복사하는 로직을 구현합니다.
  - **Tags:** `engine`, `render`, `fog`
  - **Blocked by:** #2

- **Issue #6: Fog 렌더링을 위한 ShowFlag 추가**
  - **설명:** 렌더링 디버깅 및 토글을 위해 `ShowFlag` 시스템의 기본을 구현하거나 기존 시스템에 'Fog' 플래그를 추가합니다. 에디터 UI(ImGui 등)에서 이 플래그를 켜고 끌 수 있도록 연동합니다.
  - **Tags:** `engine`, `editor`, `fog`
  - **Blocked by:** #2

- **Issue #7: `HeightFogCommon.hlsl` 셰이더 파일 설계**
  - **설명:** Fog 계산에 공통적으로 사용될 상수 버퍼 레이아웃과 유틸리티 함수들을 포함하는 `HeightFogCommon.hlsl` (또는 유사한 이름의) 셰이더 파일을 작성합니다.
  - **Tags:** `shader`, `fog`
  - **Blocked by:** #5

### ☁️ Stage 3 — Fog 렌더링 구현
*(실제 Fog 렌더링 로직 개발)*

- **Issue #8: Exponential Height Fog 픽셀 셰이더 구현**
  - **설명:** `HeightFog.hlsl`과 같은 셰이더 파일에 Exponential Height Fog를 계산하는 픽셀 셰이더 로직을 구현합니다. SceneDepth 값을 샘플링하여 월드 포지션을 복원하고, 높이에 따른 안개 농도를 계산하여 최종 색상을 결정합니다.
  - **Tags:** `shader`, `fog`
  - **Blocked by:** #5, #7

- **Issue #9: `Renderer`에 Fog 렌더링 파이프라인 연결**
  - **설명:** `Renderer::Render()` 함수 내의 적절한 시점(e.g., 투명 객체 렌더링 전)에 Fog 렌더링 패스를 호출하도록 통합합니다.
  - **Tags:** `render`, `fog`
  - **Blocked by:** #8

- **Issue #10: ShowFlag와 Fog 렌더링 연동**
  - **설명:** #6에서 구현한 Fog ShowFlag 값에 따라 #9에서 추가한 Fog 렌더링 패스의 호출 여부를 제어하도록 구현합니다.
  - **Tags:** `engine`, `render`, `fog`
  - **Blocked by:** #6, #9

### 🔥 Stage 4 — FireBall 통합 및 후처리
*(VFX 효과 구현 및 통합)*

- **Issue #11: `FireBallComponent`의 Post-Process 영향 구현**
  - **설명:** `FireBallComponent`의 위치, 반경, 색상 등의 데이터를 Post-Process 단계에서 사용할 수 있도록 GPU로 전달하는 로직을 구현합니다.
  - **Tags:** `vfx`, `post-process`
  - **Blocked by:** #3

- **Issue #12: FireBall 렌더링 통합 (Post-Process)**
  - **설명:** Post-Process 셰이더에서 FireBall의 데이터를 사용하여, 특정 반경 내에 지정된 색상의 조명 효과를 더하는 셰이더 코드를 작성하고 렌더링 파이프라인에 통합합니다.
  - **Tags:** `render`, `vfx`, `shader`
  - **Blocked by:** #11

---

## 💧 낮은 우선순위 💧

### 🎨 Stage 5 — 선택/보조 과제 (병렬 가능)

- **Issue #13: `RotationMovementComponent` 구현**
  - **설명:** `ActorComponent`를 상속받아, 소유 액터(Owner Actor)를 매 프레임 지정된 속도(`RotationRate`)로 회전시키는 `RotationMovementComponent`를 구현합니다.
  - **Tags:** `gameplay`, `component`
  - **Blocked by:** -

- **Issue #14: `ProjectileMovementComponent` 구현**
  - **설명:** `ActorComponent`를 상속받아, 소유 액터를 지정된 속도와 방향으로 이동시키는 `ProjectileMovementComponent`를 구현합니다. 중력, 탄성 등 간단한 물리 옵션을 포함할 수 있습니다.
  - **Tags:** `gameplay`, `component`
  - **Blocked by:** -

- **Issue #15: FXAA 픽셀 셰이더 구현**
  - **설명:** #4에서 구축한 Post-Process 패스 위에서 동작할 FXAA 픽셀 셰이더(`FXAA.hlsl`)를 작성합니다. Luma 값을 기반으로 에지를 감지하고 블러 처리하여 계단 현상을 완화합니다.
  - **Tags:** `fxaa`, `shader`
  - **Blocked by:** #4

- **Issue #16: FXAA Post-Process 통합**
  - **설명:** #15에서 작성한 FXAA 셰이더를 Post-Process 파이프라인에 최종적으로 통합하고, 활성화/비활성화 할 수 있는 옵션을 추가합니다.
  - **Tags:** `fxaa`, `render`
  - **Blocked by:** #15

### 🧪 Stage 6 — QA / 튜닝 / 통합 테스트

- **Issue #17: Fog 파라미터 QA 및 튜닝**
  - **설명:** 에디터에서 `HeightFogComponent`의 파라미터(Density, Falloff, Color 등)를 변경하면서 시각적으로 의도한 대로 동작하는지 검증하고 튜닝합니다.
  - **Tags:** `test`, `qa`, `fog`
  - **Blocked by:** #10

- **Issue #18: FireBall 파라미터 QA 및 튜닝**
  - **설명:** `FireBallComponent`의 파라미터(반경, 강도, 감쇠 곡선 등)를 조정하며 시각적 결과물을 검증하고 튜닝합니다.
  - **Tags:** `test`, `qa`, `vfx`
  - **Blocked by:** #12

- **Issue #19: 최종 비주얼 통합 테스트**
  - **설명:** Fog, FireBall, FXAA가 모두 활성화된 상태에서 비주얼이 깨지거나 예상치 못한 상호작용이 발생하는지 종합적으로 테스트합니다.
  - **Tags:** `test`, `qa`
  - **Blocked by:** #17, #18, #16
