-- MissilePool.lua: 미사일 오브젝트 풀 관리 (싱글톤)
-- 미사일을 재사용하여 생성/삭제 오버헤드 감소

setmetatable(_ENV, { __index = EngineTypes })

local FVector = EngineTypes.FVector

-- ==================================================
-- 싱글톤 풀 인스턴스
-- ==================================================
local MissilePoolInstance = nil

local MissilePool = {}

-- ==================================================
-- 초기화
-- ==================================================
function MissilePool:Initialize()
    local pool = {
        InactiveMissiles = {},  -- 비활성 미사일 풀
        ActiveMissiles = {},    -- 활성 미사일 추적
        PoolSize = 0,           -- 총 생성된 미사일 수
        MaxPoolSize = 50,       -- 최대 풀 크기
        MissileSpeed = 300.0,   -- 미사일 속도
        MissileLifeTime = 5.0,  -- 미사일 생존 시간
        MissileScale = 30.0,    -- 미사일 크기
    }

    setmetatable(pool, { __index = MissilePool })
    MissilePoolInstance = pool

    Print("[MissilePool] Initialized (PoolSize: " .. pool.PoolSize .. ")")
    return pool
end

-- ==================================================
-- 싱글톤 인스턴스 가져오기
-- ==================================================
function MissilePool.GetInstance()
    if not MissilePoolInstance then
        return MissilePool:Initialize()
    end
    return MissilePoolInstance
end

-- ==================================================
-- 풀 완전 초기화 (PIE 재시작 시 호출)
-- ==================================================
function MissilePool.Reset()
    if MissilePoolInstance then
        Print("[MissilePool] Resetting pool (clearing " .. MissilePoolInstance.PoolSize .. " missiles)")
        MissilePoolInstance.InactiveMissiles = {}
        MissilePoolInstance.ActiveMissiles = {}
        MissilePoolInstance.PoolSize = 0
    end
    MissilePoolInstance = nil
end

-- ==================================================
-- 미사일 가져오기 (없으면 새로 생성)
-- ==================================================
function MissilePool:GetMissile(Location, Rotation)
    local World = GetWorld()
    if not World then
        Print("[MissilePool] ERROR: World is nil!")
        return nil
    end

    local MissileActor = nil

    -- 풀에서 비활성 미사일 찾기
    if #self.InactiveMissiles > 0 then
        MissileActor = table.remove(self.InactiveMissiles)
        Print("[MissilePool] Reusing missile from pool (Pool size: " .. #self.InactiveMissiles .. ")")
    else
        -- 풀이 비어있으면 새로 생성
        if self.PoolSize >= self.MaxPoolSize then
            Print("[MissilePool] WARNING: Pool is full! Consider increasing MaxPoolSize")
            return nil
        end

        MissileActor = World:SpawnActor("AStaticMeshActor")
        if not MissileActor then
            Print("[MissilePool] ERROR: Failed to spawn missile!")
            return nil
        end

        -- 미사일 메쉬 로드 (GetStaticMeshComponent 헬퍼 사용)
        local MeshComp = MissileActor:GetStaticMeshComponent()

        if MeshComp then
            MeshComp:SetStaticMesh("Data/Missile.obj")
            Print("[MissilePool] Missile mesh loaded: Data/Missile.obj")
        else
            Print("[MissilePool] ERROR: Failed to get StaticMeshComponent!")
        end

        -- 미사일 초기 설정 (한 번만)
        MissileActor:SetActorScale3D(FVector(self.MissileScale, self.MissileScale, self.MissileScale))

        -- Lua 스크립트 연결
        MissileActor:SetUseScript(true)
        MissileActor:InitLuaScriptComponent()
        local LuaComp = MissileActor:GetLuaScriptComponent()
        if LuaComp then
            LuaComp:SetScriptName("Missile")
        end

        self.PoolSize = self.PoolSize + 1
        Print("[MissilePool] Created new missile (Total created: " .. self.PoolSize .. ")")
    end

    -- 미사일 위치/회전 설정
    MissileActor.ActorLocation = Location
    MissileActor.ActorRotation = Rotation

    -- 활성 미사일 목록에 추가
    table.insert(self.ActiveMissiles, MissileActor)

    return MissileActor
end

-- ==================================================
-- 미사일 풀로 반환
-- ==================================================
function MissilePool:ReturnMissile(MissileActor)
    if not MissileActor then
        Print("[MissilePool] ERROR: ReturnMissile called with nil actor!")
        return
    end

    local actorName = MissileActor:GetName()
    local found = false

    -- 활성 목록에서 제거 (이름으로 비교)
    for i = #self.ActiveMissiles, 1, -1 do
        local actor = self.ActiveMissiles[i]
        if actor and actor:GetName() == actorName then
            table.remove(self.ActiveMissiles, i)
            found = true
            break
        end
    end

    if not found then
        Print("[MissilePool] WARNING: Missile " .. actorName .. " not found in ActiveMissiles!")
    end

    -- 미사일 비활성화 (화면 밖으로 이동)
    MissileActor.ActorLocation = FVector(0, 0, -10000)

    -- 비활성 풀에 추가
    table.insert(self.InactiveMissiles, MissileActor)

    Print("[MissilePool] Missile returned to pool: " .. actorName .. " (Inactive: " .. #self.InactiveMissiles .. ", Active: " .. #self.ActiveMissiles .. ")")
end

-- ==================================================
-- 디버그 정보 출력
-- ==================================================
function MissilePool:PrintStatus()
    Print("[MissilePool] Status - Total: " .. self.PoolSize .. ", Active: " .. #self.ActiveMissiles .. ", Inactive: " .. #self.InactiveMissiles)
end

return MissilePool
