-- ============================================================================
-- Camera Transition Test Script
-- 언리얼 스타일 카메라 트랜지션 테스트
-- ============================================================================

return function()
    local ReturnTable = {}

    -- 카메라 전환 테스트 포인트들
    local testPoints = {
        {
            name = "Front View",
            location = FVector(500, 0, 100),
            rotation = FRotator(0, 180, 0),
            ease = ECameraEaseType.EaseInOut
        },
        {
            name = "Top View",
            location = FVector(0, 0, 800),
            rotation = FRotator(-80, 0, 0),
            ease = ECameraEaseType.SmoothStep
        },
        {
            name = "Side View",
            location = FVector(0, 600, 100),
            rotation = FRotator(0, -90, 0),
            ease = ECameraEaseType.EaseOut
        },
        {
            name = "Low Angle",
            location = FVector(300, 300, 50),
            rotation = FRotator(-10, -135, 0),
            ease = ECameraEaseType.SmootherStep
        },
        {
            name = "High Angle",
            location = FVector(-400, 0, 600),
            rotation = FRotator(-45, 0, 0),
            ease = ECameraEaseType.EaseIn
        }
    }

    local currentPointIndex = 1
    local transitionDuration = 2.0
    local isAutoPlaying = false
    local autoPlayTimer = 0.0
    local autoPlayInterval = 3.0  -- 각 뷰를 3초 동안 보여줌

    function ReturnTable:BeginPlay()
        Print("===========================================")
        Print("Camera Transition Test - Started!")
        Print("===========================================")
        Print("Controls:")
        Print("  1-5: Jump to specific camera view")
        Print("  SPACE: Cycle through all views")
        Print("  A: Start auto-play mode")
        Print("  S: Stop transitions")
        Print("  Q/E: Adjust transition duration")
        Print("  R: Reset to first view")
        Print("===========================================")

        -- 첫 번째 뷰로 즉시 이동
        self:TransitionToPoint(1, 0.0)
    end

    function ReturnTable:Tick(dt)
        local input = GetInputManager()
        local world = GetWorld()
        if not world then return end

        local camMgr = world:GetPlayerCameraManager()
        if not camMgr then return end

        -- Auto-play mode
        if isAutoPlaying then
            autoPlayTimer = autoPlayTimer + dt

            if autoPlayTimer >= autoPlayInterval then
                autoPlayTimer = 0.0
                currentPointIndex = currentPointIndex + 1
                if currentPointIndex > #testPoints then
                    currentPointIndex = 1
                end
                self:TransitionToPoint(currentPointIndex, transitionDuration)
            end
        end

        -- Manual controls (숫자 키: 1~5)
        if input:IsKeyPressed("Num1") then
            self:TransitionToPoint(1, transitionDuration)
            isAutoPlaying = false
        elseif input:IsKeyPressed("Num2") then
            self:TransitionToPoint(2, transitionDuration)
            isAutoPlaying = false
        elseif input:IsKeyPressed("Num3") then
            self:TransitionToPoint(3, transitionDuration)
            isAutoPlaying = false
        elseif input:IsKeyPressed("Num4") then
            self:TransitionToPoint(4, transitionDuration)
            isAutoPlaying = false
        elseif input:IsKeyPressed("Num5") then
            self:TransitionToPoint(5, transitionDuration)
            isAutoPlaying = false
        end

        -- Space: Cycle through views
        if input:IsKeyPressed("Space") then
            currentPointIndex = currentPointIndex + 1
            if currentPointIndex > #testPoints then
                currentPointIndex = 1
            end
            self:TransitionToPoint(currentPointIndex, transitionDuration)
            isAutoPlaying = false
        end

        -- A: Auto-play mode
        if input:IsKeyPressed("A") then
            isAutoPlaying = not isAutoPlaying
            autoPlayTimer = 0.0
            if isAutoPlaying then
                Print("Auto-play mode: ON")
                self:TransitionToPoint(currentPointIndex, transitionDuration)
            else
                Print("Auto-play mode: OFF")
            end
        end

        -- S: Stop transition
        if input:IsKeyPressed("S") then
            camMgr:StopCameraTransition()
            isAutoPlaying = false
            Print("Transition stopped")
        end

        -- Q/E: Adjust duration
        if input:IsKeyPressed("Q") then
            transitionDuration = transitionDuration - 0.5
            if transitionDuration < 0.5 then
                transitionDuration = 0.5
            end
            Print("Transition duration: " .. string.format("%.1f", transitionDuration) .. "s")
        elseif input:IsKeyPressed("E") then
            transitionDuration = transitionDuration + 0.5
            if transitionDuration > 5.0 then
                transitionDuration = 5.0
            end
            Print("Transition duration: " .. string.format("%.1f", transitionDuration) .. "s")
        end

        -- R: Reset
        if input:IsKeyPressed("R") then
            currentPointIndex = 1
            self:TransitionToPoint(1, transitionDuration)
            isAutoPlaying = false
            Print("Reset to first view")
        end

        -- Display current status
        if camMgr:IsCameraTransitioning() then
            local progress = camMgr:GetTransitionProgress()
            -- Progress bar (매 프레임마다 출력하면 너무 많으니까 생략)
        end
    end

    function ReturnTable:TransitionToPoint(index, duration)
        local world = GetWorld()
        if not world then return end

        local camMgr = world:GetPlayerCameraManager()
        if not camMgr then return end

        local point = testPoints[index]
        if not point then return end

        Print("-------------------------------------------")
        Print("Transitioning to: " .. point.name)
        Print("  Location: (" .. string.format("%.0f, %.0f, %.0f",
            point.location.X, point.location.Y, point.location.Z) .. ")")
        Print("  Duration: " .. string.format("%.1f", duration) .. "s")
        Print("  Easing: " .. self:GetEaseTypeName(point.ease))
        Print("-------------------------------------------")

        camMgr:StartTransitionToLocation(
            point.location,
            point.rotation,
            duration,
            point.ease
        )

        currentPointIndex = index
    end

    function ReturnTable:GetEaseTypeName(easeType)
        if easeType == ECameraEaseType.Linear then return "Linear"
        elseif easeType == ECameraEaseType.EaseIn then return "EaseIn"
        elseif easeType == ECameraEaseType.EaseOut then return "EaseOut"
        elseif easeType == ECameraEaseType.EaseInOut then return "EaseInOut"
        elseif easeType == ECameraEaseType.SmoothStep then return "SmoothStep"
        elseif easeType == ECameraEaseType.SmootherStep then return "SmootherStep"
        else return "Unknown"
        end
    end

    return ReturnTable
end
