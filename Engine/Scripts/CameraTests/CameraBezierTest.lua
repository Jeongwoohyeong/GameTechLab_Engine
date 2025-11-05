-- CameraBezierTest.lua
-- 베지어 곡선을 사용한 카메라 트랜지션 테스트

return function()
    local ReturnTable = {}

    local world = nil
    local camMgr = nil
    local input = nil

    -- 다양한 베지어 프리셋
    local bezierPresets = {
        {
            name = "EaseOutQuad",
            cp = {0.25, 0.46, 0.45, 0.94}
        },
        {
            name = "EaseInQuad",
            cp = {0.55, 0.085, 0.68, 0.53}
        },
        {
            name = "EaseInOutQuad",
            cp = {0.455, 0.03, 0.515, 0.955}
        },
        {
            name = "EaseOutCubic",
            cp = {0.215, 0.61, 0.355, 1.0}
        },
        {
            name = "EaseInCubic",
            cp = {0.55, 0.055, 0.675, 0.19}
        },
        {
            name = "EaseInOutCubic",
            cp = {0.645, 0.045, 0.355, 1.0}
        },
        {
            name = "EaseOutElastic",
            cp = {0.68, -0.55, 0.265, 1.55}  -- 탄성 효과
        }
    }

    local currentPresetIndex = 1

    -- 카메라 뷰 포인트
    local testPoints = {
        {
            name = "Front View",
            location = FVector(600, 0, 200),
            rotation = FRotator(-20, 180, 0)
        },
        {
            name = "Top View",
            location = FVector(0, 0, 800),
            rotation = FRotator(-80, 0, 0)
        },
        {
            name = "Side View",
            location = FVector(0, 600, 100),
            rotation = FRotator(-10, -90, 0)
        },
        {
            name = "Low Angle",
            location = FVector(300, 300, 50),
            rotation = FRotator(10, -135, 0)
        },
        {
            name = "High Angle",
            location = FVector(-400, 0, 600),
            rotation = FRotator(-60, 0, 0)
        }
    }

    local currentPointIndex = 1
    local transitionDuration = 2.0

    function ReturnTable:BeginPlay()
        Print("=== Camera Bezier Transition Test Started ===")

        -- 주의: GetWorld(), GetInputManager() 함수를 사용해야 합니다
        Print("=== Checking Lua bindings...")

        Print("=== Controls ===")
        Print("  [1-5 Keys] - Transition to specific view")
        Print("  [SPACE] - Cycle through all views")
        Print("  [B] - Cycle Bezier preset")
        Print("  [S] - Stop transition")
        Print("  [Q/E] - Adjust speed")
        Print("  [R] - Reset to first view")
        Print("")
        Print("Current Bezier: " .. bezierPresets[currentPresetIndex].name)
    end

    function ReturnTable:TransitionToPoint(index, duration)
        local world = GetWorld()
        if not world then return end

        local camMgr = world:GetPlayerCameraManager()
        if not camMgr then return end

        local point = testPoints[index]
        currentPointIndex = index

        -- 현재 선택된 베지어 프리셋 적용
        local preset = bezierPresets[currentPresetIndex]
        camMgr:SetTransitionBezierControlPoints(
            preset.cp[1], preset.cp[2], preset.cp[3], preset.cp[4]
        )

        Print("-------------------------------------------")
        Print("Transitioning to: " .. point.name)
        Print("  Location: (" .. point.location.X .. ", " .. point.location.Y .. ", " .. point.location.Z .. ")")
        Print("  Duration: " .. duration .. "s")
        Print("  Bezier: " .. preset.name)
        Print("  Control Points: (" .. preset.cp[1] .. ", " .. preset.cp[2] .. ", " .. preset.cp[3] .. ", " .. preset.cp[4] .. ")")
        Print("-------------------------------------------")

        camMgr:StartTransitionToLocation(
            point.location,
            point.rotation,
            duration,
            ECameraEaseType.Bezier  -- 베지어 모드 사용
        )
    end

    function ReturnTable:CycleBezierPreset()
        currentPresetIndex = currentPresetIndex + 1
        if currentPresetIndex > #bezierPresets then
            currentPresetIndex = 1
        end

        local preset = bezierPresets[currentPresetIndex]
        Print("===========================================")
        Print("Bezier Preset Changed: " .. preset.name)
        Print("  Control Points: (" .. preset.cp[1] .. ", " .. preset.cp[2] .. ", " .. preset.cp[3] .. ", " .. preset.cp[4] .. ")")
        Print("===========================================")
    end

    function ReturnTable:Tick(dt)
        local input = GetInputManager()
        if not input then return end

        local world = GetWorld()
        if not world then return end

        local camMgr = world:GetPlayerCameraManager()
        if not camMgr then return end

        -- Manual controls (숫자 키: 1~5)
        if input:IsKeyPressed("Num1") then
            self:TransitionToPoint(1, transitionDuration)
        elseif input:IsKeyPressed("Num2") then
            self:TransitionToPoint(2, transitionDuration)
        elseif input:IsKeyPressed("Num3") then
            self:TransitionToPoint(3, transitionDuration)
        elseif input:IsKeyPressed("Num4") then
            self:TransitionToPoint(4, transitionDuration)
        elseif input:IsKeyPressed("Num5") then
            self:TransitionToPoint(5, transitionDuration)
        end

        -- SPACE: Cycle through views
        if input:IsKeyPressed("Space") then
            currentPointIndex = currentPointIndex + 1
            if currentPointIndex > #testPoints then
                currentPointIndex = 1
            end
            self:TransitionToPoint(currentPointIndex, transitionDuration)
        end

        -- B: Cycle Bezier preset
        if input:IsKeyPressed("B") then
            self:CycleBezierPreset()
        end

        -- S: Stop transition
        if input:IsKeyPressed("S") then
            camMgr:StopCameraTransition()
            Print("[Stopped] Camera transition stopped")
        end

        -- Q/E: Adjust speed
        if input:IsKeyPressed("Q") then
            transitionDuration = transitionDuration + 0.5
            Print("[Speed] Duration: " .. transitionDuration .. "s (slower)")
        elseif input:IsKeyPressed("E") then
            transitionDuration = math.max(0.5, transitionDuration - 0.5)
            Print("[Speed] Duration: " .. transitionDuration .. "s (faster)")
        end

        -- R: Reset to first view
        if input:IsKeyPressed("R") then
            currentPointIndex = 1
            self:TransitionToPoint(1, transitionDuration)
            Print("[Reset] Back to first view")
        end

        -- Show progress bar if transitioning
        if camMgr:IsCameraTransitioning() then
            local progress = camMgr:GetTransitionProgress()
            local barLength = 30
            local filledLength = math.floor(progress * barLength)
            local bar = string.rep("█", filledLength) .. string.rep("░", barLength - filledLength)

            -- Print progress every 0.1s (대략적으로)
            if math.random() < 0.05 then
                Print(string.format("[Progress] %s %.0f%%", bar, progress * 100))
            end
        end
    end

    return ReturnTable
end
