-- Missile.lua: 미사일의 동작을 정의합니다 (오브젝트 풀링 지원)
-- Factory Pattern으로 각 미사일마다 독립적인 인스턴스 생성

setmetatable(_ENV, { __index = EngineTypes })

local FVector = EngineTypes.FVector

-- MissilePool 로드
local MissilePoolModule = require("Scripts/MissilePool")

return function()
    local MissileActor = nil
    local ReturnTable = {}

    -- ==================================================
    -- 미사일 설정
    -- ==================================================
    ReturnTable.Speed = 3600.0              -- 미사일 속도 (6배 증가)
    ReturnTable.Damage = 10.0               -- 데미지
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
        if not self.bIsActive and CurrentLocation.Z > -1000 then
            -- 재활성화!
            self.bIsActive = true
            self.CurrentLifeTime = 0.0
            self.bHasHit = false

            -- 방향 재계산
            local Rotation = MissileActor.ActorRotation
            self.Direction = Rotation:RotateVector(FVector(1, 0, 0))
            self.Direction:Normalize()

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

        -- 앞으로 이동
        local Movement = self.Direction * self.Speed * DeltaTime
        local NewLocation = CurrentLocation + Movement
        MissileActor.ActorLocation = NewLocation
        self.LastLocation = NewLocation
    end

    -- ==================================================
    -- OnBeginOverlap: 충돌 처리 (C++ CapsuleComponent에서 호출)
    -- ==================================================
    function ReturnTable:OnBeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult)
        if not OtherActor or self.bHasHit then
            return
        end

        -- 자기 자신과의 충돌 무시
        if OtherActor == MissileActor then
            return
        end

        local OtherName = OtherActor:GetName()

        -- ✅ 다른 미사일과의 충돌 무시 (AMissileActor로 시작하는 이름)
        if string.find(OtherName, "AMissileActor") then
            return
        end

        -- ✅ 플레이어와의 충돌 무시 (APlayerCharacter로 시작하는 이름)
        if string.find(OtherName, "APlayerCharacter") then
            return
        end

        Print("[Missile] OnBeginOverlap with: " .. OtherName)

        -- 적 캐릭터인지 확인 (AEnemyCharacter만 처리)
        if string.find(OtherName, "AEnemyCharacter") then
            local OtherLuaComp = OtherActor:GetLuaScriptComponent()
            if OtherLuaComp then
                -- TakeDamage 함수 호출
                Print("[Missile] Calling TakeDamage(" .. self.Damage .. ") on " .. OtherName)
                OtherLuaComp:ActivateFunction("TakeDamage", self.Damage, MissileActor)
            else
                Print("[Missile] WARNING: Enemy has no Lua component!")
            end

            -- 충돌 시 미사일을 풀로 반환
            self.bHasHit = true
            local Pool = MissilePoolModule.GetInstance()
            if Pool then
                Pool:ReturnMissile(MissileActor)
            end
        end
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
