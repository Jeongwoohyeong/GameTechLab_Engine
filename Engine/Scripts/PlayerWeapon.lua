-- PlayerWeapon.lua: 플레이어 무기 시스템 (미사일 발사 with 오브젝트 풀링)
-- PlayerCharacter에 붙여서 사용

setmetatable(_ENV, { __index = EngineTypes })

local FVector = EngineTypes.FVector

-- MissilePool 로드
local MissilePoolModule = require("Scripts/MissilePool")

return function()
    local PlayerActor = nil
    local MissilePool = nil
    local ReturnTable = {}

    -- ==================================================
    -- 무기 설정
    -- ==================================================
    ReturnTable.FireCooldown = 0.3          -- 발사 쿨다운 (초)
    ReturnTable.CurrentCooldown = 0.0       -- 현재 쿨다운
    ReturnTable.MissileMeshPath = "Data/Missile.obj"  -- 미사일 메쉬
    ReturnTable.MissileScale = 0.5          -- 미사일 크기
    ReturnTable.SpawnOffset = 20.0          -- 발사 위치 오프셋

    -- ==================================================
    -- BeginPlay: 초기화
    -- ==================================================
    function ReturnTable:BeginPlay()
        PlayerActor = self.this
        if not PlayerActor then
            Print("[PlayerWeapon] ERROR: PlayerActor is nil!")
            return
        end

        -- PIE 재시작을 위해 풀 초기화
        MissilePoolModule.Reset()

        -- MissilePool 싱글톤 가져오기 (새로 생성)
        MissilePool = MissilePoolModule.GetInstance()
        if not MissilePool then
            Print("[PlayerWeapon] ERROR: Failed to get MissilePool instance!")
            return
        end

        local ActorName = PlayerActor:GetName()
        Print("[PlayerWeapon] Initialized for: " .. ActorName)
        Print("[PlayerWeapon] Press Left Mouse Button to fire missile!")
        Print("[PlayerWeapon] Using Object Pooling for missiles")
    end

    -- ==================================================
    -- Tick: 입력 체크 및 쿨다운 관리
    -- ==================================================
    function ReturnTable:Tick(DeltaTime)
        if not PlayerActor then
            return
        end

        -- 쿨다운 감소
        if self.CurrentCooldown > 0.0 then
            self.CurrentCooldown = self.CurrentCooldown - DeltaTime
        end

        -- 마우스 왼쪽 버튼 입력 체크
        local InputMgr = GetInputManager()
        if InputMgr and InputMgr:IsKeyPressed("MouseLeft") and self.CurrentCooldown <= 0.0 then
            self:FireMissile()
            self.CurrentCooldown = self.FireCooldown
        end
    end

    -- ==================================================
    -- FireMissile: 미사일 발사 (오브젝트 풀 사용)
    -- ==================================================
    function ReturnTable:FireMissile()
        if not MissilePool then
            Print("[PlayerWeapon] ERROR: MissilePool is nil!")
            return
        end

        -- 플레이어 위치와 회전
        local PlayerLocation = PlayerActor.ActorLocation
        local PlayerRotation = PlayerActor.ActorRotation

        -- 앞 방향 계산
        local Forward = PlayerRotation:RotateVector(FVector(1, 0, 0))
        Forward:Normalize()

        -- 미사일 생성 위치 (플레이어 앞쪽)
        local SpawnLocation = PlayerLocation + Forward * self.SpawnOffset

        -- 풀에서 미사일 가져오기 (없으면 자동으로 생성)
        local MissileActor = MissilePool:GetMissile(SpawnLocation, PlayerRotation)
        if not MissileActor then
            Print("[PlayerWeapon] Failed to get missile from pool!")
            return
        end

        Print("[PlayerWeapon] Missile fired from pool at " .. tostring(SpawnLocation.X) .. ", " .. tostring(SpawnLocation.Y) .. ", " .. tostring(SpawnLocation.Z))
    end

    -- ==================================================
    -- EndPlay
    -- ==================================================
    function ReturnTable:EndPlay()
        Print("[PlayerWeapon] EndPlay")
    end

    return ReturnTable
end
