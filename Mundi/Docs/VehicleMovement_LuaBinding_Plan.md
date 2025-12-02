# USimpleWheeledVehicleMovementComponent Lua 연동 계획

## 목표
- Lua 스크립트에서 차량 입력을 설정할 수 있도록 `USimpleWheeledVehicleMovementComponent`의 입력 API를 노출한다.
- Lua에서 액터/컴포넌트 접근 → 입력 Setter 호출 → PhysX 입력 적용까지의 경로를 확보한다.

## 작업 항목
1) **리플렉션/바인딩 대상 지정**
   - 입력 Setter(`SetThrottleInput`, `SetSteeringInput`, `SetBrakeInput`, `SetHandbrakeInput`)와 필요 시 기어 변경(`GearUp`, `GearDown`)을 `UFUNCTION`으로 선언.
   - Lua 자동 바인딩 규칙을 따르는 매크로/메타데이터 확인(`GENERATED_REFLECTION_BODY`와 호환되도록).

2) **코드젠/바인딩 재생성**
   - `.\Mundi\GenerateBindings.bat` 또는 `python Mundi\BuildTools\CodeGenerator\generate.py --source-dir Mundi\Source\Runtime --output-dir Mundi\Generated` 실행.
   - 생성된 Lua 바인딩 파일에 VehicleMovement 함수가 포함되었는지 확인.

3) **Lua 접근 경로 설계**
   - Lua에서 액터 참조를 얻는 방식 확인(예: `World:FindActorByName`, `self:GetOwner()` 등).
   - 액터에서 `GetComponent(USimpleWheeledVehicleMovementComponent::StaticClass())`로 컴포넌트를 획득하는 Lua 샘플 작성.

4) **Lua 입력 적용 샘플 작성**
   - 프레임별 업데이트 함수(예: `Tick(dt)`)에서 입력 값을 결정하고 Setter 호출.
   - 예시:
     - `Vehicle:SetThrottleInput(1.0)` 전진
     - `Vehicle:SetSteeringInput(0.5)` 좌/우 조향
     - `Vehicle:SetBrakeInput(1.0)` 풀 브레이크
     - `Vehicle:SetHandbrakeInput(1.0)` 핸드브레이크
   - 입력 범위/클램프(-1~1, 0~1)를 Lua 주석으로 명확히 기재.

5) **동기화/틱 순서 확인**
   - 현재 입력 → PhysScene에서 `Simulate` 진입 직전에 `ApplyInputToPhysX` 적용됨.
   - Lua 입력 호출 시점이 그 이전(예: 게임 틱 중)임을 보장하도록 Lua 호출 타이밍 문서화.

6) **간단한 검증**
   - Lua 스크립트에서 전/후진, 좌/우 조향, 브레이크 토글을 수행해 차량이 움직이는지 수동 검증.
   - 로그 또는 온스크린 디버그로 현재 입력 값을 출력하는 임시 코드/스크립트 추가 가능.

## 산출물
- `USimpleWheeledVehicleMovementComponent` 입력 Setter의 Lua 바인딩.
- Lua 예제 스크립트(전/후진, 조향, 브레이크 테스트).
- 실행/재생성 절차가 적힌 간단한 노트(옵션).
