# AngryCoach

2인 로컬 대전 격투 게임

[![Gameplay Video](https://img.youtube.com/vi/-dkKVFIBj4k/0.jpg)](https://www.youtube.com/watch?v=-dkKVFIBj4k)

> 클릭하여 게임플레이 영상 보기

---

## 게임 개요

**AngryCoach**는 2인 로컬 대전 격투 게임입니다. 각 플레이어는 다양한 악세서리를 장착하고 약공격, 강공격, 스킬을 조합하여 상대를 쓰러뜨립니다.

### 주요 게임 시스템

| 시스템 | 설명 |
|--------|------|
| 전투 | 약공격(Light), 강공격(Heavy), 스페셜 스킬 |
| 악세서리 | Punch, Knife, Gorilla, Cloak 등 장착 가능 |
| 가드 | 방어 시스템 |
| 점프 공격 | 방향 입력 기반 공중 공격 |
| 래그돌 | 피격/사망 시 물리 기반 반응 |
| 데칼 페인팅 | 캐릭터별 데칼 생성 |

---

## 게임 클래스 (AngryCoach 접두어)

| 클래스 | 설명 |
|--------|------|
| `AAngryCoachCharacter` | 플레이어 캐릭터 - 전투, 스킬, 악세서리 시스템 관리 |
| `AAngryCoachGameMode` | 게임 모드 - 2인 대전 규칙, 플레이어 HP 관리 |
| `AAngryCoachPlayerController` | 입력 처리 및 플레이어 제어 |

---

## Mundi 엔진

C++로 개발된 커스텀 게임 엔진

### 그래픽스

| 항목 | 사양 |
|------|------|
| API | **DirectX 11** |
| 좌표계 | Z-Up, Left-Handed |
| 버텍스 순서 | CW (시계 방향) |

**렌더링 기능:**
- Deferred/Forward 렌더링
- PBR 머티리얼
- 라이팅 (Ambient, Directional, Point, Spot, FakeSpot)
- Height Fog
- Decal 시스템
- 파티클 시스템
- 텍스트 렌더링
- 빌보드

### 물리 엔진

| 라이브러리 | 용도 |
|------------|------|
| **NVIDIA PhysX** | 리지드바디 물리, 충돌, 래그돌 |
| **NvCloth** | 옷감/천 시뮬레이션 |

### 애니메이션

- 스켈레탈 애니메이션
- 애니메이션 몽타주
- BlendSpace 1D/2D
- 애니메이션 노티파이 (파티클, 사운드 등)
- IK 시스템

### 에디터

- **ImGui** 기반 UI
- 노드 기반 블루프린트 에디터 (imgui-node-editor)
- 기즈모 시스템 (이동, 회전, 스케일)
- 그리드 액터
- 선택 매니저
- 클립보드 매니저

### 스크립팅

- **Lua** 스크립팅 (Sol 바인딩)
- 게임 로직, UI 등 Lua로 제어 가능

### 에셋 파이프라인

| 포맷 | 설명 |
|------|------|
| FBX | 메시, 스켈레톤, 애니메이션 임포트 (FBXSDK) |
| OBJ | 스태틱 메시 임포트 (Right→Left Handed 자동 변환) |
| DirectXTex | 텍스처 로딩/변환 |

---

## Third Party 라이브러리

| 라이브러리 | 버전/용도 |
|------------|-----------|
| PhysX | 물리 엔진 |
| NvCloth | 옷감 시뮬레이션 |
| FBX SDK | FBX 임포트 |
| ImGui | 에디터 UI |
| imgui-node-editor | 블루프린트 노드 에디터 |
| DirectXTex | 텍스처 처리 |
| DirectXTK | DirectX 툴킷 |
| Lua | 스크립팅 언어 |
| Sol | Lua C++ 바인딩 |
| nlohmann/json | JSON 파싱 |
| CrashHandler | 크래시 덤프 처리 |

---

## 엔진 아키텍처

```
Mundi/
├── Source/
│   ├── Runtime/           # 런타임 코어
│   │   ├── Core/          # Object, Math, Memory, Container
│   │   ├── Engine/        # Animation, Components, Physics, GameFramework
│   │   ├── Renderer/      # D3D11 렌더러
│   │   └── AssetManagement/  # 메시, 텍스처 로더
│   ├── Editor/            # 에디터 기능
│   │   ├── BlueprintGraph/   # 블루프린트 시스템
│   │   ├── FBX/              # FBX 로더
│   │   └── Gizmo/            # 기즈모 컴포넌트
│   ├── Game/              # 게임 로직 (AngryCoach)
│   └── Slate/             # UI 위젯
├── Generated/             # 리플렉션 자동 생성 코드
└── ThirdParty/            # 서드파티 라이브러리
```

---

## 빌드 환경

- **IDE:** Visual Studio 2022
- **언어:** C++17
- **플랫폼:** Windows x64

---

## 좌표계 규칙

> **경고:** 이 규칙은 엔진 렌더링의 근본입니다. 수정 시 좌표계 및 버텍스 연산이 깨집니다.

| 구분 | Mundi 엔진 | OBJ Import 전 | OBJ Import 후 |
|------|------------|---------------|---------------|
| 좌표계 | Z-Up, Left-Handed | Z-Up, Right-Handed | Z-Up, Left-Handed |
| 버텍스 순서 | CW (시계 방향) | CCW (반시계 방향) | CW |

**Blender Export 설정:** Z-Up, X-Forward
