-- EnemyCharacter.lua
-- 적 캐릭터 스크립트 (데미지 처리 포함)

return function()
    local ReturnTable = {}

    -- 적 캐릭터 HP
    local maxHP = 100
    local currentHP = maxHP

    function ReturnTable:BeginPlay()
        Print("[EnemyCharacter Lua] BeginPlay - HP: " .. currentHP .. "/" .. maxHP)
    end

    function ReturnTable:Tick(dt)
        -- AI 로직이나 이동 로직을 여기에 추가 가능
    end

    -- 데미지를 받는 함수
    function ReturnTable:TakeDamage(damage, attacker)
        if currentHP <= 0 then
            return  -- 이미 죽은 상태
        end

        currentHP = currentHP - damage
        Print("[EnemyCharacter Lua] TakeDamage: -" .. damage .. " HP, Remaining: " .. currentHP .. "/" .. maxHP)

        if currentHP <= 0 then
            currentHP = 0
            Print("[EnemyCharacter Lua] Enemy destroyed!")
            self:OnDeath()
        end
    end

    -- 죽었을 때 처리
    function ReturnTable:OnDeath()
        Print("[EnemyCharacter Lua] OnDeath called")

        -- 스코어 추가
        local uiManager = GetGameUIManager()
        if uiManager then
            local hudWidget = uiManager:GetHUDWidget()
            if hudWidget then
                hudWidget:AddScore(100)  -- 적 처치 시 100점 추가
                Print("[EnemyCharacter Lua] Score added: +100 (Current: " .. hudWidget:GetScore() .. ")")
            else
                Print("[EnemyCharacter Lua] WARNING: HUD Widget not found")
            end
        else
            Print("[EnemyCharacter Lua] WARNING: GameUIManager not found")
        end

        -- TODO: 폭발 이펙트, 아이템 드롭 등

        -- 액터 삭제 (World에서 제거)
        -- 주의: Lua에서 직접 액터 삭제는 크래쉬 위험이 있을 수 있음
        -- C++에서 처리하거나, 이벤트 방식으로 처리 권장

        -- 임시: 화면 밖으로 이동
        local pos = self.this.ActorLocation
        self.this.ActorLocation = FVector(pos.X, pos.Y, -10000)
    end

    function ReturnTable:OnBeginOverlap(otherActor)
        -- 다른 오버랩 처리가 필요하면 여기에 추가
    end

    return ReturnTable
end
