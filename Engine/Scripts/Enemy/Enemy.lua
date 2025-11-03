setmetatable(_ENV, { __index = EngineTypes })

local FVector = EngineTypes.FVector

return function()
    local ReturnTable = {
        lastDamageTime = 0,    -- 마지막 데미지 시간
        damageCooldown = 1.0   -- 쿨다운 시간 (초)
    }
    Print("EnemyEnemyEnemyEnemyEnemyEnemy")

    -- 데미지를 입힐 수 있는지 체크
    function ReturnTable:CanDealDamage()
        local timeManager = GetTimeManager()
        if not timeManager then
            return false
        end

        local currentTime = timeManager:GetGameTime()
        local timeSinceLastDamage = currentTime - self.lastDamageTime

        return timeSinceLastDamage >= self.damageCooldown
    end

    -- 데미지 타이머 갱신
    function ReturnTable:UpdateDamageTime()
        local timeManager = GetTimeManager()
        if timeManager then
            self.lastDamageTime = timeManager:GetGameTime()
        end
    end

    function ReturnTable:OnBeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult)
        Print("[Lua/Enemy] Begin overlap")

        -- 쿨다운 체크
        if not self:CanDealDamage() then
            return
        end

        -- OtherActor를 PlayerCharacter로 캐스팅 시도
        if OtherActor then
            local playerChar = Cast_APlayerCharacter(OtherActor)

            if playerChar then
                Print("[Lua/Enemy] Overlap with PlayerCharacter!")

                -- GameUIManager를 통해 HUD 가져오기
                local uiManager = GetGameUIManager()
                if uiManager then
                    local hudWidget = uiManager:GetHUDWidget()
                    if hudWidget then
                        -- HUD의 TakeDamage 함수 호출 (10 데미지)
                        hudWidget:TakeDamage(10)
                        Print("[Lua/Enemy] Dealt 10 damage to player HUD. Current health: " .. hudWidget:GetHealth())

                        -- 카메라 쉐이크 효과 (강도: 2.0, 지속시간: 0.5초)
                        playerChar:StartCameraShake(2.0, 0.5)
                        Print("[Lua/Enemy] Camera shake started")

                        -- 데미지 시간 갱신
                        self:UpdateDamageTime()
                    else
                        Print("[Lua/Enemy] HUD Widget not found")
                    end
                else
                    Print("[Lua/Enemy] GameUIManager not found")
                end
            end
        end
    end

    function ReturnTable:OnEndOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex)
        Print("[Lua/Enemy] End overlap")
    end

    function ReturnTable:OnHit(OverlappedComp, OtherActor, OtherComp, NormalImpulse, OutHit)
        Print("[Lua/Enemy] Hit")

        -- 쿨다운 체크
        if not self:CanDealDamage() then
            Print("[Lua/Enemy] On cooldown, ignoring hit")
            return
        end

        -- OtherActor를 PlayerCharacter로 캐스팅 시도
        if OtherActor then
            local playerChar = Cast_APlayerCharacter(OtherActor)

            if playerChar then
                Print("[Lua/Enemy] Hit PlayerCharacter!")

                -- GameUIManager를 통해 HUD 가져오기
                local uiManager = GetGameUIManager()
                if uiManager then
                    local hudWidget = uiManager:GetHUDWidget()
                    if hudWidget then
                        -- HUD의 TakeDamage 함수 호출 (10 데미지)
                        hudWidget:TakeDamage(10)
                        Print("[Lua/Enemy] Dealt 10 damage to player HUD. Current health: " .. hudWidget:GetHealth())

                        -- 카메라 쉐이크 효과 (강도: 2.0, 지속시간: 0.5초)
                        playerChar:StartCameraShake(2.0, 0.5)
                        Print("[Lua/Enemy] Camera shake started")

                        -- 데미지 시간 갱신
                        self:UpdateDamageTime()
                    else
                        Print("[Lua/Enemy] HUD Widget not found")
                    end
                else
                    Print("[Lua/Enemy] GameUIManager not found")
                end
            end
        end
    end

    return ReturnTable
end