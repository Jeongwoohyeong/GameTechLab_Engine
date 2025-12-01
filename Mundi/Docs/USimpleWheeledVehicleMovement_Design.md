# USimpleWheeledVehicleMovementComponent 설계 정리

자동차를 **스켈레탈 메쉬 + 레이캐스트 차량(PhysX Vehicle)** 구조로 구현하는 것을 목표로 한 설계 정리.

핵심 아이디어는:

- 에셋 레벨에서는  
  **`USkeletalMesh` + `UPhysicsAsset` + `UBodySetup`** 만으로  
  “차체 바디 하나 + 휠 본(bone) 4개” 구조를 정의하고,
- 런타임에서는  
  **`USkeletalMeshComponent`의 `FBodyInstance`가 만든 `PxRigidDynamic` 하나**를  
  **`PxVehicleDrive4W::setup()`에 그대로 넘겨서** 차량을 구성하며,
- 휠은 **RigidBody가 아니라 “차체 기준의 휠 데이터 + 본 애니메이션”**으로만 표현한다.

---

## 1. 전체 구조 개요

### 에셋 레벨

- **USkeletalMesh (CarMesh)**
  - 스켈레톤 본 구조:
    - Root: `Chassis`
    - Child bones: `Wheel_FL`, `Wheel_FR`, `Wheel_RL`, `Wheel_RR`
  - `UPhysicsAsset* PhysicsAsset;` 포인터를 통해 기본 물리 에셋을 참조

- **UPhysicsAsset (CarPhysicsAsset)**
  - 여러 `UBodySetup`를 가진 컨테이너
  - 자동차용으로 최소 구성:
    - `Chassis` 본에 매핑된 BodySetup 1개 (실제 시뮬레이션 바디)
    - 휠 본에 대해서는 기본적으로 BodySetup **없음** (필요 시 별도 옵션)

- **UBodySetup**
  - 한 개 물리 바디의 **충돌 형상(AggGeom)** + **기본 물리 파라미터** 정의  
  - `UBodySetupCore` 안에 `BoneName` 등을 두어 “어느 본에 붙는 바디인지”를 표시

### 런타임 레벨

- **USkeletalMeshComponent (차량 메쉬 컴포넌트)**
  - `USkeletalMesh* SkeletalMesh` 사용
  - `FBodyInstance BodyInstance`  
    → `UBodySetup`를 기반으로 `PxRigidDynamic + PxShape들`을 생성/관리

- **USimpleWheeledVehicleMovementComponent**
  - `UMovementComponent` 파생
  - `UpdatedComponent` 로 차량 메쉬 컴포넌트를 가리킴
  - 내부에 `PxVehicleDrive4W* Vehicle`
  - `TArray<FWheelSetup>` 으로 휠 설정(BoneName, Radius, 등)을 보관
  - **BodyInstance가 만든 `PxRigidDynamic` 하나를 가져와 `Vehicle->setup()`에 넘김**

---

## 2. 에셋 레벨 설계

### 2.1 USkeletalMesh – 자동차 스켈레탈 메쉬

```cpp
class USkeletalMesh : public USkinnedAsset
{
public:
    // 그래픽/스켈레톤 데이터 (본, 버텍스, 스킨 웨이트 등)
    USkeleton* Skeleton;

    // 이 메쉬에 기본으로 사용할 물리 에셋
    UPhysicsAsset* PhysicsAsset;
};
```

- 본 구조 예:
  - `Chassis` (Root)
    - `Wheel_FL`
    - `Wheel_FR`
    - `Wheel_RL`
    - `Wheel_RR`
- 아티스트/디자이너는
  - 모델링 툴에서 휠 위치를 직관적으로 배치
  - 메쉬 교체 시에도 **같은 본 이름 규칙**만 지키면, Vehicle 코드 수정 없이 교체 가능

**디자인 의도**

- _그래픽/애니메이션_ 과 _물리_ 를 느슨하게 결합:
  - 메쉬가 바뀌어도, 같은 이름의 본과 유사한 스케일이면 기존 Vehicle 세팅 재사용 가능
  - 게임 코드에서는 “`Wheel_FL`이라는 본이 있다”만 알면 됨

---

### 2.2 UPhysicsAsset – 본별 BodySetup 묶음

```cpp
class UPhysicsAsset : public UObject
{
public:
    TArray<UBodySetup*> BodySetups;     // 각 본(bone)별 바디 설계도
    // (필요 시) 관절/Constraint 정의 등
};
```

자동차에서는 일반적으로:

- `Chassis` 본용 BodySetup 1개를 **실제 시뮬레이션 바디**로 사용
  - Box/Convex 등으로 차체 전체를 감싸는 형태
  - 질량(mass), 관성(inertia), 물리 재질(마찰, 탄성) 등을 기본값으로 설정
- 휠 본용 BodySetup은 기본적으로 **두지 않는다**
  - 레이캐스트 차량에서는 휠은 “수학적 데이터 + 본 애니메이션”이기 때문에  
    **RigidBody/Shape가 필요 없음**
  - 필요하다면 나중에 “휠이 떨어져 나가 굴러다진다” 같은 연출용으로  
    별도 액터를 스폰하는 방향이 더 관리가 쉽다.

**디자인 의도 / 최적화 포인트**

- **RigidBody 하나(차체)**만 실제 시뮬레이션 → Solver 부하 감소
- PhysicsAsset은 **여러 역할(캐릭터, 차량 등)** 이 공유하기 좋은 구조  
  (여러 차량 인스턴스가 같은 PhysicsAsset을 참조)

---

### 2.3 UBodySetup – 단일 바디 설계도

```cpp
class UBodySetup : public UBodySetupCore
{
public:
    // 차체라면 Box/Convex 등 몇 개 안 되는 단순 도형들의 합
    FAggregateGeom AggGeom;

    // 기본 바디 파라미터 (질량, 콜리전 설정 등)
    FBodyInstance DefaultInstance;
};
```

- `AggGeom` 에 Box/Convex 를 1~2개만 두어 단순화
  - 차체 콜리전이 복잡하면 충돌 비용이 늘어남
  - 레이캐스트 차량은 휠 접지는 Vehicle이 별도로 처리하므로  
    차체 콜리전은 “대략적인 박스/볼륨”이면 충분한 경우가 많음
- `DefaultInstance` 에서
  - `bSimulatePhysics = true`
  - CollisionEnabled = Physics Only / Query and Physics 등
  - PhysMaterial(마찰, 탄성 등) 설정

**디자인 의도**

- **UBodySetup = 설계도(에셋)**  
- **FBodyInstance = 그 설계도를 기반으로 한 런타임 인스턴스**

---

## 3. 런타임 물리 구조

### 3.1 UPrimitiveComponent, FBodyInstance, UBodySetup

기본 구조:

```cpp
class UPrimitiveComponent : public USceneComponent
{
public:
    FBodyInstance BodyInstance;

    virtual UBodySetup* GetBodySetup() const = 0;
};
```

- `USkeletalMeshComponent` / `UStaticMeshComponent` 가 이를 상속
- `CreatePhysicsState()` 시점에:
  1. `UBodySetup* BodySetup = GetBodySetup()`
  2. `BodyInstance.InitBody(BodySetup, ComponentToWorld, this, PhysScene)`

`FBodyInstance::InitBody()` 내부에서:

- `PxRigidDynamic` 또는 `PxRigidStatic` 생성
- `UBodySetup::AggGeom`을 순회하며 각 도형에 맞는 `PxShape`를 만들고 Attach
- `PxScene::addActor()` 로 씬에 등록

자동차 액터에서는:

- `USkeletalMeshComponent` 가 `Chassis` BodySetup을 돌려주도록 구성
- `BodyInstance` 가 **차체용 `PxRigidDynamic` 하나**를 관리

---

## 4. USimpleWheeledVehicleMovementComponent 설계

### 4.1 기본 구조

```cpp
class USimpleWheeledVehicleMovementComponent : public UMovementComponent
{
public:
    // 이 컴포넌트가 조종할 차량 메쉬 (UpdatedComponent로 설정)
    // UMovementComponent에서 제공:
    // UPrimitiveComponent* UpdatedComponent;

    // PhysX Vehicle
    PxVehicleDrive4W* Vehicle;

    // 에디터/데이터로부터 설정되는 휠 정보
    TArray<FWheelSetup> WheelSetups;

    void InitVehiclePhysX();
    void TickComponent(float DeltaTime);
};
```

- `UpdatedComponent` 는 `USkeletalMeshComponent` 또는 `UStaticMeshComponent` 를 기대
- `WheelSetups` 안에는:
  - `FName BoneName;` (예: "Wheel_FL")
  - `float WheelRadius;`
  - `bool bIsDrivenWheel;` 등

### 4.2 InitVehiclePhysX – 같은 액터를 Vehicle에 넘기기

핵심 로직 요약:

```cpp
void USimpleWheeledVehicleMovementComponent::InitVehiclePhysX()
{
    // 1) UpdatedComponent에서 BodyInstance 가져오기
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(UpdatedComponent);
    FBodyInstance& BodyInstance = PrimComp->BodyInstance;

    // 2) BodyInstance가 생성한 PxRigidDynamic 얻기
    PxRigidDynamic* ChassisActor = BodyInstance.GetPxRigidDynamic();

    // 3) 휠/드라이브 데이터 세팅
    PxVehicleWheelsSimData* WheelsSimData = CreateWheelsSimDataFromWheelSetups(PrimComp);
    PxVehicleDriveSimData4W DriveData = CreateDriveSimData();

    // 4) Vehicle 객체 allocate + setup
    PxVehicleDrive4W* Drive4W = PxVehicleDrive4W::allocate(WheelCount);
    Drive4W->setup(*Physics, ChassisActor, *WheelsSimData, DriveData, WheelCount);

    Vehicle = Drive4W;
}
```

여기서 **중요한 포인트**:

- `ChassisActor`는 이미 `BodyInstance.InitBody()` 에 의해 만들어져,  
  **씬에 `addActor`된 상태인 하나의 RigidBody**다.
- `Drive4W->setup(...)`에 **그 RigidBody 포인터 그대로**를 넘긴다:
  - Vehicle은 이 RigidBody를 “내 차체”로 인식하고,
  - 이후 업데이트에서 이 바디에 힘/토크/속도를 적용한다.
- 새로운 RigidDynamic을 Vehicle용으로 따로 만들지 않는다.

**디자인 의도**

- **RigidBody의 “소유자”는 항상 FBodyInstance / 컴포넌트**
- Vehicle 컴포넌트는 그 바디를 **조종하는 컨트롤러**일 뿐
- 이렇게 해야:
  - 충돌/트레이스/이벤트 등은 계속 `BodyInstance` 기준으로 일관되게 들어오고
  - Vehicle을 끄더라도 기존 물리 구조는 그대로 유지 가능

---

## 5. 휠 설계 – 본(bone) + 레이캐스트

### 5.1 휠에 BodySetup이 없어도 되는 이유

레이캐스트 차량에서:

- **차체 RigidBody 하나만 존재**
- 휠은:
  - **Raycast**로 지면 접지를 구하고
  - 접지 결과로부터 **차체에** 힘/토크를 가한다
- 휠용 RigidBody/Shape는 필요 없다

따라서:

- `UPhysicsAsset`에서 휠 본에는 BodySetup을 두지 않아도 된다
- **휠 본은 “시각화용 뼈”에 가깝고**,  
  Vehicle 시뮬레이션에서 사용하는 건 **휠 위치/반지름 등의 데이터**뿐

### 5.2 휠 위치 데이터 – 차체 기준 로컬 좌표

휠의 위치 데이터는 **차체 RigidBody 로컬 좌표계 기준**으로 들어가야 한다.

좌표계 예:

- X: 앞(+), 뒤(-)
- Y: 오른쪽(+), 왼쪽(-)
- Z: 위(+), 아래(-)

왼쪽 앞바퀴 위치 예:

```text
WheelCentreOffset_FL = ( +앞쪽_거리, -좌측_오프셋, -아래쪽_높이 )
```

**계산 플로우 예시**

1. `USkeletalMeshComponent`에서 휠 본의 컴포넌트 공간 위치를 얻는다
   - `FTransform BoneCompTransform = SkelComp->GetBoneTransform(BoneIndex, EBoneSpaces::ComponentSpace);`
2. 차체 BodyInstance의 월드 트랜스폼을 구한다
   - `FTransform ChassisWorld = BodyInstance.GetUnrealWorldTransform();`
3. `ChassisWorld`의 역변환을 적용하여 **차체 로컬 공간으로 변환**
   - `WheelLocal = BoneCompWorld * ChassisWorld.Inverse();`
4. `WheelLocal`의 위치를 `WheelCentreOffset` 으로 사용

이렇게 하면:

- 메쉬/본 배치가 변경되어도 자동차 로직은 자연스럽게 따라감
- “휠 위치 = 차체 기준 상대 위치” 라는 전제가 항상 유지됨

---

## 6. 생명주기(Lifecycle)

1. **액터 생성 / 컴포넌트 등록**
   - `USkeletalMeshComponent::CreatePhysicsState()`  
     → `BodyInstance.InitBody(ChassisBodySetup, ...)`  
     → `PxRigidDynamic` + `PxShape` 생성 후 `PxScene::addActor`
2. **VehicleMovementComponent 활성화**
   - `InitVehiclePhysX()` 호출
   - `BodyInstance`에서 `PxRigidDynamic*`를 얻고  
     → `PxVehicleDrive4W::setup(...)` 에 넘김
3. **매 프레임 업데이트**
   - 입력 처리 → `PxVehicleDrive4W` / `PxVehicleWheels` 업데이트
   - PhysX 시뮬레이션 → `PxScene::simulate()` / `fetchResults()`
   - `FBodyInstance`가 최종 바디 트랜스폼을 가져와 컴포넌트 Transform에 반영
   - 휠 본 트랜스폼을 Vehicle 결과(서스펜션 압축, 바퀴 회전각)에 맞춰 갱신
4. **파괴 / 언로드**
   - 먼저 `PxVehicleDrive4W::free()` / `Vehicle = nullptr`
   - 이후 `BodyInstance.TermBody()` → `PxScene::removeActor` + `PxRigidDynamic::release()`
   - 컴포넌트/액터 파괴

---

## 7. 실무/최적화 관점에서의 요약

- **RigidBody는 한 번만 만들 것**
  - Vehicle용, 일반 물리용 따로 만들지 말고 **BodyInstance가 만든 바디 하나를 재사용**
- **에셋과 런타임의 역할 분리**
  - `UBodySetup` / `UPhysicsAsset` : “설계도 (공유 에셋)”
  - `FBodyInstance` : “실제 물리 바디 (인스턴스)”
  - `USimpleWheeledVehicleMovementComponent` : “그 바디를 차량처럼 운전하는 컨트롤러”
- **휠은 본 + 데이터로만 처리**
  - “휠 바디 4개 + 조인트” 구조는 설계/디버깅이 복잡하고 성능도 손해
  - 대부분의 게임 차량은 **레이캐스트 + 차체 한 바디** 구조로 충분
- **본 기반 설계의 장점**
  - 아트 리소스 교체에 강함 (본 이름만 지키면 됨)
  - 코드 쪽에서는 항상 “차체 기준 휠 상대 위치”만 신경 쓰면 됨

---

## 8. 한 줄로 정리

> **USkeletalMesh + UPhysicsAsset(차체 BodySetup 1개)** 로 에셋을 정의하고,  
> `USkeletalMeshComponent::BodyInstance`가 만든 `PxRigidDynamic` 하나를  
> `USimpleWheeledVehicleMovementComponent`의 `PxVehicleDrive4W::setup()`에 그대로 넘겨서  
> “같은 액터(바디)”를 일반 물리와 차량 물리 양쪽에서 함께 쓰는 구조가 이 설계의 핵심이다.
