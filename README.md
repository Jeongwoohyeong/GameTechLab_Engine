# GTL Engine (Week 04)

이 프로젝트는 C++와 DirectX 11을 기반으로 한 3D 게임 엔진입니다. Actor-Component 시스템을 갖추고 있으며, ImGui를 활용한 디버깅 및 에디터 기능을 포함하고 있습니다.

---

## ✨ 주요 기능 (Features)

- DirectX 11 기반 3D 렌더링
- Actor-Component 아키텍처
- JSON 파일을 이용한 레벨(월드) 로딩 및 저장
- ImGui를 활용한 실시간 에디터 및 디버깅 UI
- 3D 모델(.obj) 및 텍스처(.dds) 로딩

---

## 💻 코드 구조 (Code Structure)

- **.obj 파일 파싱 (.obj File Parsing)**
  - `FObjManager` 클래스(`Utility/ObjManager.h`에 구현)를 사용하여 `.obj` 형식의 3D 모델 파일을 파싱합니다.
  - 이 매니저는 파일에서 정점(vertex), 인덱스(index), 텍스처 좌표(UV) 데이터를 읽어와 `StaticMesh` 객체 생성을 위한 데이터를 제공합니다.

- **StaticMesh**
  - 파싱된 모델 데이터를 기반으로 렌더링에 필요한 Vertex Buffer와 Index Buffer를 생성하고 관리하는 클래스입니다.
  - `MaterialList`를 통해 multi-material을 구현합니다.

- **다중 뷰포트 (Multi-Viewport)**
  - `Editor/Viewport.h`에 정의된 `Viewport` 클래스를 통해 렌더링된 씬을 표시합니다.
  - 현재 구현은 단일 뷰포트를 중심으로 하지만, 여러 `Viewport` 객체를 생성하여 화면에 다중 뷰포트를 구성할 수 있는 구조를 가지고 있습니다.

- **TObjectIterator**
  - `Core/Object.h`에 구현된 템플릿 클래스로, 특정 `UObject` 파생 클래스의 모든 인스턴스를 순회하는 기능을 제공합니다.
  - 예를 들어 `for (UStaticMesh* pMesh : TObjectIterator<UStaticMesh>())` 와 같은 형태로 로드된 `.obj` 정보를 포함하는 모든 `StaticMesh` 객체를 쉽게 탐색하고 처리할 수 있습니다.

---

## 🛠️ 사용된 기술 (Tech Stack)

- **언어 (Language):** C++
- **개발 환경 (IDE):** Visual Studio 2022
- **그래픽스 API (Graphics API):** DirectX 11
- **주요 라이브러리 (Libraries):**
  - `DirectXTK`: DirectX 11 개발을 위한 헬퍼 라이브러리
  - `ImGui`: 디버깅 및 에디터 UI
  - `simplejson`: JSON 데이터 처리

---

## 🚀 시작하기 (Getting Started)

### 요구사항 (Requirements)

- **OS:** Windows 10 이상
- **IDE:** Visual Studio 2022 (`C++를 사용한 데스크톱 개발` 워크로드)
- **SDK:** Windows 11 SDK

### 빌드 순서 (Build Steps)

1.  이 저장소를 클론합니다.
2.  루트 디렉토리의 `GTL.sln` 파일을 Visual Studio 2022로 엽니다.
3.  솔루션 탐색기에서 `Engine` 프로젝트를 우클릭하고 **빌드(Build)**를 선택합니다.
4.  빌드가 성공하면 `Build/Debug/` 디렉토리에 `Engine.exe` 파일이 생성됩니다.
5.  `Build/Debug/Engine.exe`를 직접 실행하거나, Visual Studio에서 `로컬 Windows 디버거`로 프로젝트를 시작합니다.

---

## 📂 프로젝트 구조 (Project Structure)

GTLWeek04/<br>
├── Engine/         # 엔진 핵심 소스 코드<br>
├── External/       # 외부 라이브러리 (DirectXTK, json 등)<br>
├── Document/       # 레벨 파일 예시 등 관련 문서<br>
├── Build/          # 빌드 결과물 (실행 파일, 에셋 등)<br>
└── GTL.sln         # Visual Studio 솔루션 파일<br>