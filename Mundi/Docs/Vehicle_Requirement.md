`USimpleWheeledVehicleMovementComponent`을 이용해 4륜 자동차의 움직임을 구현하는 로직이 다음과 같이 이루어지길 기대하고 있습니다.

## 애셋
- 자동차의 메시 애셋은 스켈레탈 메시(`USkeltalMesh`)이다.
- 자동차 스켈레탈 메시의 스켈레탈은 차체 본을 root로 하고 4개의 자식 본을 가진다. 각 자식 본은 바퀴에 대응된다.
- 스켈레탈 메시 애셋은 사용하는 PhysicsAsset을 가진다.
- PhysicsAsset은 차체 bone 위치에 하나의 body setup을 가진다. 이 body setup이 물리 시뮬레이션을 적용받을 차체가 된다.

## 액터 / 컴포넌트 / 초기화
- 스켈레탈 메시 액터(`USkeletalMeshActor`)를 만든다. 스켈레탈 메시 액터의 root component는 스켈레탈 메시 컴포넌트(`USkeletalMeshComponent`)이다.
- 스켈레탈 메시 컴포넌트가 사용할 스켈레탈 메시를 위에서 준비한 자동차 애셋으로 설정한다.

- `USimpleWheeledVehicleMovementComponent`는 월드에 등록될 때 유효한 UpdatedComponent를 찾아 설정한다. (위와 같이 스켈레탈 메시 액터에 부착하면 루트 컴포넌트가 지정됨)
- 월드에 등록될 때 월드의 PhysScene에도 함께 등록한다.