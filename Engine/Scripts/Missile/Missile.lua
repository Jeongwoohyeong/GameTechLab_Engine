-- Missile.lua: 미사일의 동작을 정의합니다 (오브젝트 풀링 지원)
-- Factory Pattern으로 각 미사일마다 독립적인 인스턴스 생성

setmetatable(_ENV, { __index = EngineTypes })

local FVector = EngineTypes.FVector

-- MissilePool 로드
local MissilePoolModule = require("Scripts/Missile/MissilePool")

return function()
    local MissileActor = nil
    local ReturnTable = {}

    -- ==================================================
    -- 미사일 설정
    -- ==================================================
    ReturnTable.Speed = 3600.0              -- 미사일 속도 (6배 증가)
    ReturnTable.Damage = 100.0               -- 데미지
    ReturnTable.LifeTime = 5.0              -- 최대 생존 시간 (초)
    ReturnTable.CurrentLifeTime = 0.0       -- 현재 생존 시간
    ReturnTable.bHasHit = false             -- 충돌 플래그
    ReturnTable.bIsActive = false           -- 활성화 플래그 (풀링용)
    ReturnTable.Direction = FVector(1, 0, 0) -- 이동 방향
    ReturnTable.LastLocation = FVector(0, 0, -10000) -- 마지막 위치 (변경 감지용)

    -- ==================================================
    -- BeginPlay: 미사일 생성 시 호출 (풀에서 처음 생성될 때만)
    -- ==================================================
    function ReturnTable:BeginPlay()
        MissileActor = self.this
        if not MissileActor then
            Print("[Missile] ERROR: MissileActor is nil!")
            return
        end

        Print("[Missile] BeginPlay called (initial creation)")

        -- LastLocation을 매우 먼 곳으로 설정하여 첫 Tick에서 활성화 감지되도록
        self.LastLocation = FVector(0, 0, -10000)
    end

    -- ==================================================
    -- Tick: 매 프레임 호출
    -- ==================================================
    function ReturnTable:Tick(DeltaTime)
        if not MissileActor then
            return
        end

        local CurrentLocation = MissileActor.ActorLocation

        -- 비활성 상태에서 위치가 정상 범위로 돌아왔는지 체크 (풀에서 재활성화)
        if not self.bIsActive and CurrentLocation.Z > -10000 then
            -- 재활성화!
            self.bIsActive = true
            self.CurrentLifeTime = 0.0
            self.bHasHit = false

            -- 방향 재계산
            local Rotation = MissileActor.ActorRotation
            self.Direction = Rotation:RotateVector(FVector(1, 0, 0))
            self.Direction:Normalize()

            -- C++ 멤버 변수에 방향과 속도 설정 (EnemyBaseActor가 사용)
            MissileActor.MissileDirection = self.Direction
            MissileActor.MissileSpeed = self.Speed

            self.LastLocation = CurrentLocation
            Print("[Missile] Reactivated from pool: " .. MissileActor:GetName())
        end

        -- 비활성 상태면 Tick 중지
        if not self.bIsActive then
            return
        end

        -- 생존 시간 체크
        self.CurrentLifeTime = self.CurrentLifeTime + DeltaTime
        if self.CurrentLifeTime >= self.LifeTime then
            Print("[Missile] Lifetime expired, returning to pool...")

            -- 풀로 반환
            local Pool = MissilePoolModule.GetInstance()
            if Pool then
                Pool:ReturnMissile(MissileActor)
            end
            self.bIsActive = false
            return
        end

        -- C++ 멤버 변수 업데이트 (매 프레임 - EnemyBaseActor가 실시간으로 사용)
        MissileActor.MissileDirection = self.Direction
        MissileActor.MissileSpeed = self.Speed

        -- 앞으로 이동
        local Movement = self.Direction * self.Speed * DeltaTime
        local NewLocation = CurrentLocation + Movement
        MissileActor.ActorLocation = NewLocation
        self.LastLocation = NewLocation
    end

    -- ==================================================
    -- OnOverlap: 충돌 발생 시 호출
    -- ==================================================
    function ReturnTable:OnOverlap(OtherActor)
        if not self.bIsActive then return end
        if not OtherActor then return end
        if self.bHasHit then return end  -- 이미 충돌했으면 무시

        -- Hit Stop 효과 트리거 (0.15초 프리즈로 타격감 강화)
        local TimeManager = GetTimeManager()
        TimeManager:StartHitStop(0.15)

        -- 충돌 플래그 설정
        self.bHasHit = true

        Print(string.format("[Missile] Hit %s! Hit Stop triggered!", OtherActor:GetName()))

        -- 미사일을 풀로 반환
        local Pool = MissilePoolModule.GetInstance()
        if Pool then
            Pool:ReturnMissile(MissileActor)
        end
        self.bIsActive = false
    end

    -- ==================================================
    -- EndPlay: 미사일 삭제 시 호출
    -- ==================================================
    function ReturnTable:EndPlay()
        if MissileActor then
            Print("[Missile] EndPlay: " .. MissileActor:GetName())
        end
    end

    return ReturnTable
end