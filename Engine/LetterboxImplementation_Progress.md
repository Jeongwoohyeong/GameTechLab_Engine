# Letterbox 포스트 프로세싱 구현 진행사항

## 프로젝트 개요
- **목표**: DirectX 11 기반 게임 엔진에 Letterbox 포스트 프로세싱 효과 구현
- **핵심 기술**: 인덱스 기반 핑퐁 렌더링 (PingPongA ↔ PingPongB)
- **주요 기능**:
  - 런타임 종횡비 조정 (21:9, 16:9, 2.39:1 등)
  - 상하단 검은색 바 생성
  - ActiveViewportRect 기반 정확한 렌더링 영역 설정
  - FXAA와의 연계 (선택적)

---

## 구현 완료 항목

### ✅ 1. 핑퐁 렌더 타겟 생성 (DeviceResources)
- **파일**: `Engine/Source/Render/Renderer/Private/DeviceResources.cpp`
- **내용**:
  - PingPongTextureA, PingPongTextureB 생성 (R16G16B16A16_FLOAT)
  - RTV 및 SRV 생성 (각각 인덱스 0, 1로 접근)
  - 해상도 변경 시 자동 재생성 지원

### ✅ 2. LetterboxShader.hlsl 작성
- **파일**: `Engine/Asset/Shader/LetterboxShader.hlsl`
- **내용**:
  - 종횡비 기반 검은색 바 계산
  - UV 리매핑으로 중앙 영역만 렌더링
  - 상수 버퍼: TargetAspectRatio, RenderTargetWidth, RenderTargetHeight
  - **수정 완료**: 종횡비 공식 수정 (CurrentAspect / TargetAspect)

### ✅ 3. FLetterboxPass 구현
- **파일**:
  - `Engine/Source/Render/RenderPass/Public/LetterboxPass.h`
  - `Engine/Source/Render/RenderPass/Private/LetterboxPass.cpp`
- **내용**:
  - 풀스크린 쿼드 렌더링
  - 입력/출력 텍스처 설정 (SetInputTexture, SetOutputRenderTarget)
  - 종횡비 설정 (SetTargetAspectRatio)
  - **수정 완료**: ActiveViewportRect 기반 뷰포트 설정

### ✅ 4. Renderer.cpp 포스트 프로세싱 체인 통합
- **파일**: `Engine/Source/Render/Renderer/Private/Renderer.cpp`
- **내용**:
  - **인덱스 기반 핑퐁 로직 구현**:
    - PassCount 계산으로 활성화된 패스 개수 파악
    - CurrentPassIndex로 입출력 타겟 자동 계산
    - `PingPong[(CurrentPassIndex - 1) % 2]` → `PingPong[CurrentPassIndex % 2]`
    - 마지막 패스는 자동으로 백버퍼로 출력
  - 복사 패스: 패스가 없을 때 SceneColor → 백버퍼
  - FXAA OFF 상태에서도 정상 렌더링 보장

### ✅ 5. FXAAPass 뷰포트 수정
- **파일**: `Engine/Source/Render/RenderPass/Private/FXAAPass.cpp`
- **내용**:
  - UpdateConstants()에서 ActiveViewportRect 기반 해상도 계산
  - SetRenderTargets()에서 ActiveViewportRect 기반 뷰포트 설정
  - 메뉴바, 상태바, 우측 패널 영역을 제외한 정확한 렌더링

### ✅ 6. UI 컨트롤 추가
- **파일**: `Engine/Source/Render/UI/Private/Widget/MainBarWidget.cpp`
- **내용**:
  - Letterbox 토글 스위치
  - 종횡비 슬라이더 (1.0 ~ 3.0)
  - 프리셋 버튼: 16:9, 21:9, 2.39:1 (시네마스코프)

### ✅ 7. Level.h ShowFlags 수정
- **파일**: `Engine/Source/Level/Public/Level.h`
- **내용**:
  - SF_FXAA 기본 플래그 제거 (사용자 요청)
  - FXAA 없이도 정상 작동하도록 보장

---

## 핵심 해결 사항

### 🔧 문제 1: 검은 화면 (FXAA 없이 실행 시)
- **원인**: 포스트 프로세싱 체인에서 PassCount == 0일 때 렌더링 경로 누락
- **해결**: 복사 패스 추가 (SceneColor → 백버퍼, 종횡비 = 현재 화면 비율)

### 🔧 문제 2: 종횡비 공식 오류
- **원인**: `BarHeight = (1.0 - TargetAspect / CurrentAspect) * 0.5` (역순)
- **해결**: `BarHeight = (1.0 - CurrentAspect / TargetAspect) * 0.5`로 수정

### 🔧 문제 3: 메뉴바/상태바 영역이 하늘색으로 표시됨
- **원인**: 백버퍼가 ClearColor로 클리어되지 않음
- **해결**: RenderBegin()에서 백버퍼를 검은색으로 클리어 추가

### 🔧 문제 4: 위젯에 가려지지 않는 부분에 물체가 렌더링되지 않음
- **원인**: SceneColor는 ActiveViewportRect 크기로 렌더링되지만, 포스트 프로세싱이 전체 백버퍼 뷰포트 사용
- **사용자 피드백**: "래터박스로 줄이다보면 더이상 위젯에 가려지지 않는데도 그리지 않는 문제가 있잖아"
- **해결**: FXAA/LetterboxPass 모두 ActiveViewportRect 기반 뷰포트 설정으로 SceneColor 크기와 일치시킴

### 🔧 문제 5: 분기 기반 구현
- **원인**: if/else 분기로 입출력 설정 (확장성 부족)
- **해결**: 인덱스 기반 핑퐁 로직으로 리팩토링
  ```cpp
  int32 CurrentPassIndex = 0;
  // 각 패스마다 CurrentPassIndex 증가
  // 입력: PingPong[(CurrentPassIndex - 1) % 2]
  // 출력: PingPong[CurrentPassIndex % 2] (마지막 패스는 nullptr = 백버퍼)
  ```

### 🔧 문제 6: RenderBegin() 조건부 렌더링
- **원인**: FXAA OFF 시 SceneColor 대신 백버퍼로 직접 렌더링
- **해결**: FXAA 상태와 무관하게 항상 SceneColor로 렌더링하도록 수정

---

## 기술적 세부사항

### 핑퐁 렌더링 흐름
```
SceneColor → [FXAA (선택)] → [Letterbox (선택)] → 백버퍼
               ↓                    ↓
           PingPong[0]          PingPong[1] (또는 백버퍼)
```

### 인덱스 계산 로직
```cpp
// 예시 1: FXAA만 활성화 (PassCount = 1)
// CurrentPassIndex = 0, IsLastPass = true
// FXAA: SceneColor → 백버퍼 (OutputRTV = nullptr)

// 예시 2: Letterbox만 활성화 (PassCount = 1)
// CurrentPassIndex = 0, IsLastPass = true
// Letterbox: SceneColor → 백버퍼

// 예시 3: FXAA + Letterbox 모두 활성화 (PassCount = 2)
// FXAA: CurrentPassIndex = 0, IsLastPass = false
//   → SceneColor → PingPong[0]
// Letterbox: CurrentPassIndex = 1, IsLastPass = true
//   → PingPong[0] → 백버퍼
```

### ActiveViewportRect 계산
`ViewportManager.cpp:133`에서 계산:
```cpp
const int MenuAndLevelHeight = MenuBarHeight + LevelBarHeight - 12;
const int StatusBarHeight = GetStatusBarHeight();
const int32 RightPanelWidth = GetRightPanelWidth();
const int32 ViewportWidth = Width - RightPanelWidth;
const int32 ViewportHeight = Height - MenuAndLevelHeight - StatusBarHeight;
ActiveViewportRect = FRect{ 0, MenuAndLevelHeight, ViewportWidth, ViewportHeight };
```

---

## 코드 스타일 및 규칙

- **언리얼 코딩 스타일 준수**: UCLASS, FStruct 명명 규칙
- **한글 주석**: 모든 주요 코드에 한글 설명 추가
- **16바이트 정렬**: 상수 버퍼 구조체에 `alignas(16)` 사용
- **핑퐁 타겟 인덱스**: 0 = PingPongA, 1 = PingPongB

---

## 빌드 및 테스트 결과

### ✅ 빌드 성공
- **구성**: Debug|x64
- **경고**: 기존 경고만 존재 (printf 포맷, size_t 변환 등)
- **실행 파일**: `Build/Debug/Engine.exe`

### 테스트 시나리오
1. **FXAA OFF, Letterbox OFF**: SceneColor → 백버퍼 (복사 패스)
2. **FXAA ON, Letterbox OFF**: SceneColor → FXAA → 백버퍼
3. **FXAA OFF, Letterbox ON**: SceneColor → Letterbox → 백버퍼
4. **FXAA ON, Letterbox ON**: SceneColor → FXAA → PingPong[0] → Letterbox → 백버퍼

### 예상 동작
- 시작 시 검은 화면 없이 정상 렌더링
- 메뉴바/상태바 영역 제외한 정확한 영역 렌더링
- Letterbox 슬라이더로 실시간 종횡비 조정 가능
- 21:9 설정 시 상하단에 검은색 바 생성

---

## 향후 확장 가능성

### 추가 가능한 포스트 프로세싱 패스
- Bloom
- Color Grading / LUT
- Depth of Field
- Motion Blur
- Vignette

### 인덱스 기반 핑퐁의 장점
- 새로운 패스 추가 시 분기 로직 수정 불필요
- PassCount만 증가하면 자동으로 핑퐁 계산
- 유지보수성 및 가독성 향상

---

---

## 핵심 수정 사항 요약

### 뷰포트 일관성 확보
**문제**: SceneColor는 ActiveViewportRect 크기 (예: 1000x600)로 렌더링되지만, 포스트 프로세싱이 전체 백버퍼 뷰포트 (예: 1920x1080)를 사용하여 UV 불일치 발생

**해결**:
1. **LetterboxPass.cpp:217-228** - UpdateConstants()
   - RenderTargetWidth/Height를 ActiveViewportRect 기준으로 계산

2. **LetterboxPass.cpp:239-261** - SetRenderTargets()
   - 뷰포트를 ActiveViewportRect 기반으로 설정
   ```cpp
   FRect ActiveRect = UViewportManager::GetInstance().GetActiveViewportRect();
   Viewport.TopLeftX = static_cast<float>(ActiveRect.Left);
   Viewport.TopLeftY = static_cast<float>(ActiveRect.Top);
   Viewport.Width = static_cast<float>(ActiveRect.Width);
   Viewport.Height = static_cast<float>(ActiveRect.Height);
   ```

3. **FXAAPass.cpp:110-123** - UpdateConstants()
   - InvResolution을 ActiveViewportRect 기준으로 계산

4. **FXAAPass.cpp:125-146** - SetRenderTargets()
   - 뷰포트를 ActiveViewportRect 기반으로 설정 (LetterboxPass와 동일한 방식)

### 렌더링 파이프라인 일관성
**문제**: FXAA OFF 시 RenderBegin()이 백버퍼로 직접 렌더링, SceneColorSRV가 비어있음

**해결**: Renderer.cpp:1103-1124
- FXAA 상태와 무관하게 **항상 SceneColor로 렌더링**
- 백버퍼를 검은색으로 클리어하여 메뉴바/상태바 영역 처리

### 결과
- Letterbox 효과로 화면을 축소해도 위젯이 가리지 않는 부분은 계속 물체가 렌더링됨
- 메뉴바/상태바 영역은 검은색으로 표시
- FXAA 없이도 정상 작동
- 모든 포스트 프로세싱 패스가 동일한 좌표계 사용

---

## 최종 핵심 수정 (3차 - 2025-11-05)

### 🔧 문제: Letterbox 축소 시 위젯으로 가려지지 않는 영역이 하늘색으로 표시
**원인**: 포스트 프로세싱 패스(FXAA, Letterbox)가 ActiveViewportRect 기반 뷰포트를 사용하여 출력을 제한함

**해결 방법**:
1. **SceneColor 렌더링**: 전체 백버퍼에 렌더링 (RenderBegin의 UpdateViewport 유지)
2. **종횡비 계산**: ActiveViewportRect 기반 (실제 3D 콘텐츠 영역)
3. **출력 뷰포트**: 전체 백버퍼 (위젯 영역까지 포함)

**수정 파일**:
- **LetterboxPass.cpp:241-262** - SetRenderTargets()
  ```cpp
  // 뷰포트 설정: 전체 백버퍼 크기로 설정
  Viewport.Width = static_cast<float>(DeviceResources->GetWidth());
  Viewport.Height = static_cast<float>(DeviceResources->GetHeight());
  ```

- **FXAAPass.cpp:110-124** - UpdateConstants()
  ```cpp
  // 전체 백버퍼 크기를 기반으로 FXAA 해상도 계산
  const float Width = static_cast<float>(DeviceResources->GetWidth());
  const float Height = static_cast<float>(DeviceResources->GetHeight());
  FXAAParams.InvResolution = FVector2(1.0f / Width, 1.0f / Height);
  ```

- **FXAAPass.cpp:126-146** - SetRenderTargets()
  ```cpp
  // 뷰포트 설정: 전체 백버퍼 크기로 설정
  Viewport.Width = static_cast<float>(DeviceResources->GetWidth());
  Viewport.Height = static_cast<float>(DeviceResources->GetHeight());
  ```

### 결과
- Letterbox가 화면을 축소하더라도, 위젯이 더 이상 가리지 않는 영역에 렌더링된 3D 콘텐츠가 정상 표시됨
- FXAA 없이도 그리드와 액터가 시작부터 정상 표시됨
- FXAA 켜도 종횡비가 변하지 않음 (전체 백버퍼 기준으로 통일)

---

## 최종 수정 일자
**2025-11-05 (3차 수정 완료)**

## 작성자
Claude Code (claude.ai/code)
