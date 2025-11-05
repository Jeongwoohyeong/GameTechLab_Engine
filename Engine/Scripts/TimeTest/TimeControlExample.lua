-- TimeControlExample.lua
-- This script demonstrates how to use Time Control features (Hit Stop, Slow Motion) from Lua
-- Attach this script to any Actor to test time control functions with keyboard inputs

setmetatable(_ENV, { __index = EngineTypes })

return function()
    local ReturnTable = {}
    local Actor = nil
    local TimeManager = nil

    -- Called when the actor begins play
    function ReturnTable:BeginPlay()
        Actor = self.this
        TimeManager = GetTimeManager()

        Print("[TimeControl] Example Actor spawned!")
        Print("[TimeControl] Press keys to test time control:")
        Print("[TimeControl]   1 = Hit Stop (0.2s)")
        Print("[TimeControl]   2 = Slow Motion (0.3x for 3s)")
        Print("[TimeControl]   3 = Stop Slow Motion")
        Print("[TimeControl]   4 = Reset Time Dilation")
        Print("[TimeControl]   5 = Set Time Scale to 0.5x")
        Print("[TimeControl]   6 = Set Time Scale to 2.0x")
    end

    -- Called every frame
    function ReturnTable:Tick(DeltaTime)
        if not Actor then return end

        local InputManager = GetInputManager()

        -- Key 1: Trigger Hit Stop
        if InputManager:IsKeyPressed("Num1") then
            Print("[TimeControl] Starting Hit Stop (0.2s)")
            TimeManager:StartHitStop(0.2)
        end

        -- Key 2: Trigger Slow Motion
        if InputManager:IsKeyPressed("Num2") then
            Print("[TimeControl] Starting Slow Motion (0.3x speed for 3s)")
            TimeManager:StartSlowMotion(0.3, 3.0)
        end

        -- Key 3: Stop Slow Motion immediately
        if InputManager:IsKeyPressed("Num3") then
            Print("[TimeControl] Stopping Slow Motion")
            TimeManager:StopSlowMotion()
        end

        -- Key 4: Reset Time Dilation to normal
        if InputManager:IsKeyPressed("Num4") then
            Print("[TimeControl] Resetting Time Dilation to 1.0")
            TimeManager:ResetTimeDilation()
        end

        -- Key 5: Set Time Scale to 0.5x (slow)
        if InputManager:IsKeyPressed("Num5") then
            Print("[TimeControl] Setting Global Time Dilation to 0.5x")
            TimeManager:SetGlobalTimeDilation(0.5)
        end

        -- Key 6: Set Time Scale to 2.0x (fast)
        if InputManager:IsKeyPressed("Num6") then
            Print("[TimeControl] Setting Global Time Dilation to 2.0x")
            TimeManager:SetGlobalTimeDilation(2.0)
        end

        -- Display current time state
        if InputManager:IsKeyPressed("T") then
            local CurrentDilation = TimeManager:GetGlobalTimeDilation()
            local IsHitStopActive = TimeManager:IsHitStopActive()
            local IsSlowMotionActive = TimeManager:IsSlowMotionActive()

            Print(string.format("[TimeControl] Current State:"))
            Print(string.format("  Time Dilation: %.2fx", CurrentDilation))
            Print(string.format("  Hit Stop Active: %s", IsHitStopActive and "YES" or "NO"))
            Print(string.format("  Slow Motion Active: %s", IsSlowMotionActive and "YES" or "NO"))
        end

        -- Move actor to demonstrate time effect (affected by time dilation)
        local CurrentLocation = Actor.ActorLocation
        Actor.ActorLocation = CurrentLocation + FVector(100, 0, 0) * DeltaTime
    end

    return ReturnTable
end
