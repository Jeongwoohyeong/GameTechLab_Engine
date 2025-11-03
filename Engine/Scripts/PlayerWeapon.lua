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
    ReturnTable.MissileScale = FVector(50.0, 200.0, 200.0)  -- 미사일 크기
    ReturnTable.WeaponEnabled = false       -- 무기 활성화 상태 (Start Game 전까지 false)

    -- 미사일 레벨 시스템 (나중에 아이템으로 업그레이드)
    ReturnTable.MissileLevel = 1            -- 현재 미사일 레벨 (1~3)
    ReturnTable.MissilesPerShot = 2         -- 레벨 1부터 2발 발사

    -- 레벨별 미사일 발사 위치 (루트 컴포넌트로부터의 상대 오프셋)
    ReturnTable.MissileSpawnOffsets = {
        -- 레벨 1: 좌우 2발
        [1] = {
            FVector(-40.0, -25.0, 15.0),  -- 왼쪽
            FVector(-40.0, 25.0, 15.0)    -- 오른쪽
        },
        -- 레벨 2: 좌우 + 중앙 3발 (나중에 확장 가능)
        [2] = {
            FVector(-2.5, -3.0, 1.3),
            FVector(-2.5, 0.0, 1.3),
            FVector(-2.5, 3.0, 1.3)
        },
        -- 레벨 3: 4발 (나중에 확장 가능)
        [3] = {
            FVector(-2.5, -3.0, 1.3),
            FVector(-2.5, -1.0, 1.3),
            FVector(-2.5, 1.0, 1.3),
            FVector(-2.5, 3.0, 1.3)
        }
    }

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
        Print("[PlayerWeapon] Weapon is DISABLED. Waiting for game to start...")
        Print("[PlayerWeapon] Using Object Pooling for missiles")
    end

    -- ==================================================
    -- EnableWeapon: 무기 활성화 (Start Game 버튼으로 호출됨)
    -- ==================================================
    function ReturnTable:EnableWeapon()
        self.WeaponEnabled = true
        Print("[PlayerWeapon] ✅ WEAPON ENABLED! Press Left Mouse Button to fire!")
    end

    -- ==================================================
    -- DisableWeapon: 무기 비활성화
    -- ==================================================
    function ReturnTable:DisableWeapon()
        self.WeaponEnabled = false
        Print("[PlayerWeapon] ⛔ WEAPON DISABLED!")
    end

    -- ==================================================
    -- Tick: 입력 체크 및 쿨다운 관리
    -- ==================================================
    function ReturnTable:Tick(DeltaTime)
        if not PlayerActor then
            return
        end

        -- 무기가 비활성화되어 있으면 발사 불가
        if not self.WeaponEnabled then
            return
        end

        -- 쿨다운 감소
        if self.CurrentCooldown > 0.0 then
            self.CurrentCooldown = self.CurrentCooldown - DeltaTime
        end

        -- 마우스 왼쪽 버튼 입력 체크 (꾹 누르면 계속 발사)
        local InputMgr = GetInputManager()
        if InputMgr and InputMgr:IsKeyDown("MouseLeft") and self.CurrentCooldown <= 0.0 then
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

        -- 현재 레벨에 맞는 오프셋 배열 가져오기
        local spawnOffsets = self.MissileSpawnOffsets[self.MissileLevel]
        if not spawnOffsets then
            Print("[PlayerWeapon] ERROR: Invalid missile level!")
            return
        end

        -- 각 오프셋 위치에서 미사일 발사
        for i, localOffset in ipairs(spawnOffsets) do
            -- 로컬 오프셋을 월드 좌표로 변환
            local worldOffset = PlayerRotation:RotateVector(localOffset)
            local spawnLocation = PlayerLocation + worldOffset

            -- 풀에서 미사일 가져오기 (없으면 자동으로 생성)
            local MissileActor = MissilePool:GetMissile(spawnLocation, PlayerRotation, self.MissileScale)
            if not MissileActor then
                Print("[PlayerWeapon] Failed to get missile from pool!")
            end
        end

        Print("[PlayerWeapon] Level " .. self.MissileLevel .. ": " .. #spawnOffsets .. " missiles fired!")
    end

    -- ==================================================
    -- UpgradeMissile: 미사일 레벨 업그레이드 (아이템으로 호출)
    -- ==================================================
    function ReturnTable:UpgradeMissile()
        if self.MissileLevel < 3 then
            self.MissileLevel = self.MissileLevel + 1
            local newMissileCount = #self.MissileSpawnOffsets[self.MissileLevel]
            Print("[PlayerWeapon] 🚀 Missile upgraded to Level " .. self.MissileLevel .. "! (" .. newMissileCount .. " missiles per shot)")
        else
            Print("[PlayerWeapon] Missile is already at MAX level!")
        end
    end

    -- ==================================================
    -- EndPlay
    -- ==================================================
    function ReturnTable:EndPlay()
        Print("[PlayerWeapon] EndPlay")
    end

    return ReturnTable
end
