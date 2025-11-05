# 레터박스 & 핑퐁 렌더링 구현 진행 상황

## 프로젝트 정보
- **프로젝트명**: FutureEngine
- **구현 기능**: 레터박스 (Letterbox) 포스트 프로세싱
- **렌더링 방식**: 핑퐁 렌더링 (Ping-Pong Rendering)
- **시작 날짜**: 2025-11-05

## 요구사항
- ✅ **핑퐁 방식**: 완전한 핑퐁 (PingPongA ↔ PingPongB) - 확장성 고려
- ✅ **레터박스 종횡비**: 런타임 조정 가능 (21:9, 16:9, 2.39:1 등)
- ✅ **바 색상**: 검은색 고정
- ✅ **향후 확장**: 비네트 효과, 감마 컬렉션 추가 예정

## 구현 단계

### Phase 1: 핑퐁 렌더 타겟 인프라 구축 ⏳
**수정 파일**: `DeviceResources.h`, `DeviceResources.cpp`

- [ ] PingPongTextureA/B 멤버 변수 추가
  - ID3D11Texture2D* PingPongTextureA
  - ID3D11Texture2D* PingPongTextureB
  - 각각의 RTV, SRV 추가
  - Format: DXGI_FORMAT_R16G16B16A16_FLOAT

- [ ] PingPong 관리 메서드 구현
  - CreatePingPongTargets()
  - ReleasePingPongTargets()
  - GetPingPongRTV(int Index)
  - GetPingPongSRV(int Index)

- [ ] OnResize()에 통합
  - 리사이즈 시 PingPong 타겟 재생성

### Phase 2: 레터박스 패스 생성 ⏳
**새 파일**: `LetterboxPass.h`, `LetterboxPass.cpp`, `LetterboxShader.hlsl`

- [ ] LetterboxShader.hlsl 작성
  - 상수 버퍼: AspectRatio, RenderTargetSize
  - UV 기반 레터박스 바 계산
  - 이미지 영역 UV 리매핑

- [ ] LetterboxPass.h 클래스 설계
  - FRenderPass 상속
  - 멤버 변수: AspectRatio, ConstantBuffer, Fullscreen VB/IB
  - 메서드: SetAspectRatio, SetInputTexture, SetOutputRenderTarget

- [ ] LetterboxPass.cpp 구현
  - Execute() 메서드
  - 셰이더 로드 및 초기화
  - 풀스크린 쿼드 렌더링

### Phase 3: Renderer 통합 ⏳
**수정 파일**: `Renderer.h`, `Renderer.cpp`

- [ ] Renderer.h 멤버 추가
  - FLetterboxPass* LetterboxPass
  - bool bLetterboxEnabled
  - float LetterboxAspectRatio

- [ ] CreateLetterboxShader() 구현
  - 셰이더 컴파일 및 로드
  - LetterboxPass 초기화

- [ ] Update() 렌더링 파이프라인 재구성
  - FXAA 출력 → PingPongA
  - Letterbox 입력: PingPongA, 출력: PingPongB
  - 최종 복사: PingPongB → 백버퍼

- [ ] OnResize() 수정
  - PingPong 타겟 재생성 호출

### Phase 4: FXAAPass 수정 ⏳
**수정 파일**: `FXAAPass.h`, `FXAAPass.cpp`

- [ ] 출력 RTV 파라미터화
  - OutputRTV 멤버 변수 추가
  - SetOutputRenderTarget() 메서드
  - SetRenderTargets() 수정

### Phase 5: ShowFlags & ImGui 통합 ⏳
**수정 파일**: `ShowFlags.h`, `Renderer.cpp`

- [ ] ShowFlags에 SF_Letterbox 추가
- [ ] ImGui 컨트롤 추가
  - Letterbox 토글
  - 종횡비 슬라이더 (1.0 ~ 3.0)
  - 프리셋 버튼: 16:9, 21:9, 2.39:1

### Phase 6: 프로젝트 설정 & 테스트 ⏳
- [ ] Engine.vcxproj에 새 파일 추가
- [ ] 프로젝트 빌드
- [ ] 컴파일 에러 수정
- [ ] 기능 테스트
  - 다양한 해상도 테스트
  - 종횡비 전환 테스트
  - FXAA와 동시 사용 테스트

## 현재 진행 상황
**진행률**: 0/6 단계 완료

**현재 작업**: Phase 1 시작 - 구현 진행 상황 기록 파일 생성 완료

## 기술 노트

### 렌더링 파이프라인 흐름
```
Scene Rendering → SceneColorTexture
                      ↓
                  FXAA Pass
                      ↓ (출력)
                 PingPongTextureA
                      ↓ (입력)
               Letterbox Pass
                      ↓ (출력)
                 PingPongTextureB
                      ↓
           [향후 추가: 비네트/감마]
                      ↓
              최종 복사 → 백버퍼
```

### 메모리 사용량
- PingPongTextureA: 1920x1080 기준 약 16MB (R16G16B16A16_FLOAT)
- PingPongTextureB: 1920x1080 기준 약 16MB
- **총 추가 메모리**: 약 32MB

### 레터박스 종횡비 계산 공식
```
CurrentAspect = RenderTargetWidth / RenderTargetHeight
BarHeight = (1.0 - TargetAspect / CurrentAspect) * 0.5

if (UV.y < BarHeight || UV.y > 1.0 - BarHeight)
    Output = Black (0, 0, 0)
else
    RemappedUV.y = (UV.y - BarHeight) / (1.0 - 2.0 * BarHeight)
    Output = Sample(InputTexture, RemappedUV)
```

## 주요 참고 파일
- FXAA 구현: `Engine\Source\Render\RenderPass\Private\FXAAPass.cpp`
- FXAA 셰이더: `Engine\Asset\Shader\FXAAShader.hlsl`
- 렌더러 파이프라인: `Engine\Source\Render\Renderer\Private\Renderer.cpp` (Update 메서드)
- 렌더 타겟 관리: `Engine\Source\Render\Renderer\Private\DeviceResources.cpp`

## 변경 이력
- **2025-11-05**: 프로젝트 시작, 구현 계획 수립
