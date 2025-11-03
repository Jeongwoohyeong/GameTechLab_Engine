setmetatable(_ENV, { __index = EngineTypes })

local FVector = EngineTypes.FVector

return function()
    local ReturnTable = {
        isOnCooldown = false,  -- 데미지 쿨다운 플래그
        damageCooldown = 1.0   -- 쿨다운 시간 (초)
    }
    Print("EnemyEnemyEnemyEnemyEnemyEnemy")

    -- 데미지 쿨다운 코루틴
    function ReturnTable:DamageCooldown()
        wait(self.damageCooldown)
        self.isOnCooldown = false
        Print("[Lua/Enemy] Cooldown finished, ready to deal damage again")
    end

    function ReturnTable:OnBeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult)
        Print("[Lua/Enemy] Begin overlap")

        -- 쿨다운 중이면 데미지 입히지 않음
        if self.isOnCooldown then
            return
        end

        -- OtherActor를 PlayerCharacter로 캐스팅 시도
        if OtherActor then
            local playerChar = Cast_APlayerCharacter(OtherActor)

            if playerChar then
                Print("[Lua/Enemy] Overlap with PlayerCharacter!")

                -- 쿨다운 시작
                self.isOnCooldown = true

                -- GameUIManager를 통해 HUD 가져오기
                local uiManager = GetGameUIManager()
                if uiManager then
                    local hudWidget = uiManager:GetHUDWidget()
                    if hudWidget then
                        -- HUD의 TakeDamage 함수 호출 (10 데미지)
                        hudWidget:TakeDamage(10)
                        Print("[Lua/Enemy] Dealt 10 damage to player HUD. Current health: " .. hudWidget:GetHealth())
                    else
                        Print("[Lua/Enemy] HUD Widget not found")
                    end
                else
                    Print("[Lua/Enemy] GameUIManager not found")
                end

                -- 쿨다운 코루틴 시작
                StartCoroutine(self, "DamageCooldown")
            end
        end
    end

    function ReturnTable:OnEndOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex)
        Print("[Lua/Enemy] End overlap")
    end

    function ReturnTable:OnHit(OverlappedComp, OtherActor, OtherComp, NormalImpulse, OutHit)
        Print("[Lua/Enemy] Hit")

        -- 쿨다운 중이면 데미지 입히지 않음
        if self.isOnCooldown then
            Print("[Lua/Enemy] On cooldown, ignoring hit")
            return
        end

        -- OtherActor를 PlayerCharacter로 캐스팅 시도
        if OtherActor then
            local playerChar = Cast_APlayerCharacter(OtherActor)

            if playerChar then
                Print("[Lua/Enemy] Hit PlayerCharacter!")

                -- 쿨다운 시작
                self.isOnCooldown = true

                -- GameUIManager를 통해 HUD 가져오기
                local uiManager = GetGameUIManager()
                if uiManager then
                    local hudWidget = uiManager:GetHUDWidget()
                    if hudWidget then
                        -- HUD의 TakeDamage 함수 호출 (10 데미지)
                        hudWidget:TakeDamage(10)
                        Print("[Lua/Enemy] Dealt 10 damage to player HUD. Current health: " .. hudWidget:GetHealth())
                    else
                        Print("[Lua/Enemy] HUD Widget not found")
                    end
                else
                    Print("[Lua/Enemy] GameUIManager not found")
                end

                -- 쿨다운 코루틴 시작
                StartCoroutine(self, "DamageCooldown")
            end
        end
    end

    return ReturnTable
end