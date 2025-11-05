-- ============================================================================
-- Camera Follow Test Script
-- 액터 추적 카메라 트랜지션 테스트
-- ============================================================================

return function()
    local ReturnTable = {}

    local isFollowing = false
    local followTarget = nil
    local transitionDuration = 1.5

    -- 다양한 카메라 오프셋 프리셋
    local offsetPresets = {
        {
            name = "Behind (Third Person)",
            offset = FVector(-300, 0, 100),
            ease = ECameraEaseType.EaseInOut
        },
        {
            name = "Side View",
            offset = FVector(0, -400, 80),
            ease = ECameraEaseType.SmoothStep
        },
        {
            name = "Top Down",
            offset = FVector(0, 0, 500),
            ease = ECameraEaseType.EaseOut
        },
        {
            name = "Low Angle",
            offset = FVector(-200, -200, 30),
            ease = ECameraEaseType.SmootherStep
        },
        {
            name = "Far Behind",
            offset = FVector(-600, 0, 150),
            ease = ECameraEaseType.EaseIn
        }
    }

    local currentPresetIndex = 1

    function ReturnTable:BeginPlay()
        Print("===========================================")
        Print("Camera Follow Test - Started!")
        Print("===========================================")
        Print("Controls:")
        Print("  F: Toggle follow mode (follow this actor)")
        Print("  1-5: Switch camera offset preset")
        Print("  SPACE: Cycle through presets")
        Print("  Q/E: Adjust transition speed")
        Print("  X: Stop following")
        Print("===========================================")
        Print("")
        Print("NOTE: Move this actor around to see")
        Print("      the camera follow smoothly!")
        Print("===========================================")
    end

    function ReturnTable:Tick(dt)
        local input = GetInputManager()
        local world = GetWorld()
        if not world then return end

        local camMgr = world:GetPlayerCameraManager()
        if not camMgr then return end

        -- F: Toggle follow mode
        if input:IsKeyPressed("F") then
            if not isFollowing then
                followTarget = self.this
                local preset = offsetPresets[currentPresetIndex]
                self:StartFollowing(preset)
                Print("Started following actor - " .. preset.name)
            else
                self:StopFollowing()
                Print("Stopped following")
            end
        end

        -- Preset selection (1-5)
        if input:IsKeyPressed("Num1") then
            self:SwitchPreset(1)
        elseif input:IsKeyPressed("Num2") then
            self:SwitchPreset(2)
        elseif input:IsKeyPressed("Num3") then
            self:SwitchPreset(3)
        elseif input:IsKeyPressed("Num4") then
            self:SwitchPreset(4)
        elseif input:IsKeyPressed("Num5") then
            self:SwitchPreset(5)
        end

        -- Space: Cycle presets
        if input:IsKeyPressed("Space") then
            currentPresetIndex = currentPresetIndex + 1
            if currentPresetIndex > #offsetPresets then
                currentPresetIndex = 1
            end
            self:SwitchPreset(currentPresetIndex)
        end

        -- Q/E: Adjust speed
        if input:IsKeyPressed("Q") then
            transitionDuration = transitionDuration - 0.25
            if transitionDuration < 0.25 then
                transitionDuration = 0.25
            end
            Print("Transition speed: " .. string.format("%.2f", transitionDuration) .. "s")
        elseif input:IsKeyPressed("E") then
            transitionDuration = transitionDuration + 0.25
            if transitionDuration > 3.0 then
                transitionDuration = 3.0
            end
            Print("Transition speed: " .. string.format("%.2f", transitionDuration) .. "s")
        end

        -- X: Stop following
        if input:IsKeyPressed("X") then
            self:StopFollowing()
            Print("Stopped following")
        end

        -- Move actor for demonstration (WASD keys when following)
        if isFollowing and input:IsKeyDown("MouseRight") then
            local loc = self.this.ActorLocation
            local moveSpeed = 200.0 * dt

            if input:IsKeyDown("W") then
                loc.X = loc.X + moveSpeed
            end
            if input:IsKeyDown("S") then
                loc.X = loc.X - moveSpeed
            end
            if input:IsKeyDown("A") then
                loc.Y = loc.Y - moveSpeed
            end
            if input:IsKeyDown("D") then
                loc.Y = loc.Y + moveSpeed
            end
            if input:IsKeyDown("Q") then
                loc.Z = loc.Z - moveSpeed
            end
            if input:IsKeyDown("E") then
                loc.Z = loc.Z + moveSpeed
            end

            self.this.ActorLocation = loc
        end
    end

    function ReturnTable:StartFollowing(preset)
        local world = GetWorld()
        if not world then return end

        local camMgr = world:GetPlayerCameraManager()
        if not camMgr then return end

        isFollowing = true
        followTarget = self.this

        Print("-------------------------------------------")
        Print("Following with preset: " .. preset.name)
        Print("  Offset: (" .. string.format("%.0f, %.0f, %.0f",
            preset.offset.X, preset.offset.Y, preset.offset.Z) .. ")")
        Print("  Duration: " .. string.format("%.2f", transitionDuration) .. "s")
        Print("-------------------------------------------")

        camMgr:StartTransitionToActor(
            followTarget,
            transitionDuration,
            preset.ease,
            preset.offset
        )
    end

    function ReturnTable:StopFollowing()
        local world = GetWorld()
        if not world then return end

        local camMgr = world:GetPlayerCameraManager()
        if not camMgr then return end

        isFollowing = false
        followTarget = nil
        camMgr:StopCameraTransition()
    end

    function ReturnTable:SwitchPreset(index)
        if index < 1 or index > #offsetPresets then
            return
        end

        currentPresetIndex = index
        local preset = offsetPresets[currentPresetIndex]

        if isFollowing and followTarget then
            self:StartFollowing(preset)
        else
            Print("Preset selected: " .. preset.name .. " (Press F to start following)")
        end
    end

    return ReturnTable
end
