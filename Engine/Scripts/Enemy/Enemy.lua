-- Enemy.lua
-- 적 캐릭터 스크립트 (데미지 처리 + 플레이어 공격)

setmetatable(_ENV, { __index = EngineTypes })

local FVector = EngineTypes.FVector

return function()
    local ReturnTable = {}

    -- 적 캐릭터 HP
    local maxHP = 100
    local currentHP = maxHP
    local lastDamageTime = 0    -- 마지막 데미지 시간
    local damageCooldown = 1.0  -- 쿨다운 시간 (초)
    local bIsDead = false       -- 죽음 상태 플래그 (중복 OnDeath 방지)

    function ReturnTable:BeginPlay()
        --Print("[Lua/Enemy] BeginPlay - HP: " .. currentHP .. "/" .. maxHP)
        bIsDead = false  -- BeginPlay 시 항상 살아있는 상태로 초기화
        currentHP = maxHP
    end

    -- ✅ 오브젝트 풀에서 재사용될 때 호출되는 함수
    function ReturnTable:ResetEnemy()
        bIsDead = false
        currentHP = maxHP
        lastDamageTime = 0
        Print("[Lua/Enemy] ResetEnemy called - Enemy revived!")
    end

    function ReturnTable:Tick(dt)
        -- AI 로직이나 이동 로직을 여기에 추가 가능
    end

    -- 데미지를 입힐 수 있는지 체크 (플레이어 공격용)
    function ReturnTable:CanDealDamage()
        local timeManager = GetTimeManager()
        if not timeManager then
            return false
        end

        local currentTime = timeManager:GetGameTime()
        local timeSinceLastDamage = currentTime - lastDamageTime

        return timeSinceLastDamage >= damageCooldown
    end

    -- 데미지 타이머 갱신
    function ReturnTable:UpdateDamageTime()
        local timeManager = GetTimeManager()
        if timeManager then
            lastDamageTime = timeManager:GetGameTime()
        end
    end

    -- 데미지를 받는 함수 (미사일에 맞을 때)
    function ReturnTable:TakeDamage(damage, attacker)
        -- ✅ 이미 죽은 상태면 데미지 무시
        if bIsDead then
            return
        end

        -- ✅ 한 대 맞으면 무조건 죽음
        currentHP = 0
        bIsDead = true
        self:OnDeath()
    end

    -- 죽었을 때 처리 (C++에서 TakeDamage 호출 시 직접 처리되므로 사용 안 함)
    function ReturnTable:OnDeath()
        -- C++에서 직접 처리:
        -- 1. 스코어 추가 (+100)
        -- 2. 화면 밖으로 이동 (Z = -10000)
        -- 3. Tick 비활성화

        -- TODO: 폭발 이펙트, 아이템 드롭 등 추가 효과는 여기서 처리 가능
    end

    -- 플레이어와 충돌 시작 (적이 플레이어에게 데미지를 줌)
    function ReturnTable:OnBeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult)
        --Print("[Lua/Enemy] OnBeginOverlap")

        -- ✅ 죽은 상태면 데미지 주지 않음
        if bIsDead then
            return
        end

        -- 쿨다운 체크
        if not self:CanDealDamage() then
            return
        end

        -- OtherActor를 PlayerCharacter로 캐스팅 시도
        if OtherActor then
            local playerChar = Cast_APlayerCharacter(OtherActor)

            if playerChar then
                --Print("[Lua/Enemy] Overlap with PlayerCharacter!")

                -- GameUIManager를 통해 HUD 가져오기
                local uiManager = GetGameUIManager()
                if uiManager then
                    local hudWidget = uiManager:GetHUDWidget()
                    if hudWidget then
                        -- HUD의 TakeDamage 함수 호출 (10 데미지)
                        hudWidget:TakeDamage(10)
                        --Print("[Lua/Enemy] Dealt 10 damage to player. Current health: " .. hudWidget:GetHealth())

                        -- 카메라 쉐이크 효과 (강도: 2.0, 지속시간: 0.5초)
                        local camMgr = GetPlayerCameraManager()
                        if camMgr then
                            camMgr:StartCameraShake(2.0, 0.5)
                        end

                        -- 데미지 시간 갱신
                        self:UpdateDamageTime()
                    end
                end
            end
        end
    end

    function ReturnTable:OnEndOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex)
        -- 오버랩 종료 처리가 필요하면 여기에 추가
    end

    -- 플레이어와 Hit 충돌 (적이 플레이어에게 데미지를 줌)
    function ReturnTable:OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit)
        --Print("[Lua/Enemy] OnHit")

        -- ✅ 죽은 상태에서는 아무것도 하지 않음
        if bIsDead then
            return
        end

        -- 쿨다운 체크
        if not self:CanDealDamage() then
            return
        end

        -- OtherActor를 PlayerCharacter로 캐스팅 시도
        if OtherActor then
            local playerChar = Cast_APlayerCharacter(OtherActor)

            if playerChar then
                --Print("[Lua/Enemy] Hit PlayerCharacter!")

                -- GameUIManager를 통해 HUD 가져오기
                local uiManager = GetGameUIManager()
                if uiManager then
                    local hudWidget = uiManager:GetHUDWidget()
                    if hudWidget then
                        -- HUD의 TakeDamage 함수 호출 (10 데미지)
                        hudWidget:TakeDamage(10)
                        --Print("[Lua/Enemy] Dealt 10 damage to player. Current health: " .. hudWidget:GetHealth())

                        -- 카메라 쉐이크 효과 (강도: 2.0, 지속시간: 0.5초)
                        local camMgr = GetPlayerCameraManager()
                        if camMgr then
                            camMgr:StartCameraShake(2.0, 0.5)
                        end

                        -- 데미지 시간 갱신
                        self:UpdateDamageTime()
                    end
                end
            end
        end
    end

    return ReturnTable
end
