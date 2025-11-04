-- EnemyCharacter.lua
-- 적 캐릭터 스크립트 (데미지 처리 포함)

return function()
    local ReturnTable = {}

    -- 적 캐릭터 HP
    local maxHP = 100
    local currentHP = maxHP
    local lastDamageTime = 0    -- 마지막 데미지 시간
    local damageCooldown = 1.0  -- 쿨다운 시간 (초)

    function ReturnTable:BeginPlay()
        --Print("[EnemyCharacter Lua] BeginPlay - HP: " .. currentHP .. "/" .. maxHP)
    end

    function ReturnTable:Tick(dt)
        -- AI 로직이나 이동 로직을 여기에 추가 가능
    end

    -- 데미지를 입힐 수 있는지 체크
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

    -- 데미지를 받는 함수
    function ReturnTable:TakeDamage(damage, attacker)
        if currentHP <= 0 then
            return  -- 이미 죽은 상태
        end

        currentHP = currentHP - damage
        --Print("[EnemyCharacter Lua] TakeDamage: -" .. damage .. " HP, Remaining: " .. currentHP .. "/" .. maxHP)

        if currentHP <= 0 then
            currentHP = 0
            --Print("[EnemyCharacter Lua] Enemy destroyed!")
            self:OnDeath()
        end
    end

    -- 죽었을 때 처리
    function ReturnTable:OnDeath()
        --Print("[EnemyCharacter Lua] OnDeath called")

        -- 스코어 추가
        local uiManager = GetGameUIManager()
        if uiManager then
            local hudWidget = uiManager:GetHUDWidget()
            if hudWidget then
                hudWidget:AddScore(100)  -- 적 처치 시 100점 추가
                --Print("[EnemyCharacter Lua] Score added: +100 (Current: " .. hudWidget:GetScore() .. ")")
            end
        end

        -- 충돌 비활성화 (죽은 적이 더 이상 충돌하지 않도록)
        local actor = self.this
        if actor then
            local sphereComp = actor:GetComponentByClass("USphereComponent")
            if sphereComp then
                sphereComp:SetCollisionEnabled(false)
            end
        end

        -- TODO: 폭발 이펙트, 아이템 드롭 등

        -- 임시: 화면 밖으로 이동
        local pos = self.this.ActorLocation
        self.this.ActorLocation = FVector(pos.X, pos.Y, -10000)
    end

    function ReturnTable:OnBeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult)
        --Print("[EnemyCharacter] OnBeginOverlap")

        -- 쿨다운 체크
        if not self:CanDealDamage() then
            return
        end

        -- OtherActor를 PlayerCharacter로 캐스팅 시도
        if OtherActor then
            local playerChar = Cast_APlayerCharacter(OtherActor)

            if playerChar then
                --Print("[EnemyCharacter] Overlap with PlayerCharacter!")

                -- GameUIManager를 통해 HUD 가져오기
                local uiManager = GetGameUIManager()
                if uiManager then
                    local hudWidget = uiManager:GetHUDWidget()
                    if hudWidget then
                        -- HUD의 TakeDamage 함수 호출 (10 데미지)
                        hudWidget:TakeDamage(10)
                        --Print("[EnemyCharacter] Dealt 10 damage to player. Current health: " .. hudWidget:GetHealth())

                        -- 카메라 쉐이크 효과 (강도: 2.0, 지속시간: 0.5초)
                        playerChar:StartCameraShake(2.0, 0.5)

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

    function ReturnTable:OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit)
        --Print("[EnemyCharacter] OnHit")

        -- 죽은 상태에서는 아무것도 하지 않음
        if currentHP <= 0 then
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
                --Print("[EnemyCharacter] Hit PlayerCharacter!")

                -- GameUIManager를 통해 HUD 가져오기
                local uiManager = GetGameUIManager()
                if uiManager then
                    local hudWidget = uiManager:GetHUDWidget()
                    if hudWidget then
                        -- HUD의 TakeDamage 함수 호출 (10 데미지)
                        hudWidget:TakeDamage(10)
                        --Print("[EnemyCharacter] Dealt 10 damage to player. Current health: " .. hudWidget:GetHealth())

                        -- 카메라 쉐이크 효과 (강도: 2.0, 지속시간: 0.5초)
                        playerChar:StartCameraShake(2.0, 0.5)

                        -- 데미지 시간 갱신
                        self:UpdateDamageTime()
                    end
                end
            end
        end
    end

    return ReturnTable
end