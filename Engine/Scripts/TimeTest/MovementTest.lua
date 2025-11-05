-- MovementTest.lua
-- Simple movement test to verify Actor can move

setmetatable(_ENV, { __index = EngineTypes })

return function()
    local ReturnTable = {}
    local Actor = nil
    local TotalTime = 0

    function ReturnTable:BeginPlay()
        Actor = self.this
        Print("[MovementTest] Actor: " .. Actor:GetName())
        Print("[MovementTest] Tick should be called every frame")

        -- Check if Actor has RootComponent
        local RootComp = Actor:GetRootComponent()
        if RootComp then
            Print("[MovementTest] RootComponent EXISTS: " .. RootComp:GetName())
        else
            Print("[MovementTest] ERROR: NO RootComponent!")
        end

        -- Set CustomTimeDilation to 1.0 for normal speed
        Actor:SetCustomTimeDilation(1.0)
        Print("[MovementTest] CustomTimeDilation set to 1.0x")
    end

    function ReturnTable:Tick(DeltaTime)
        if not Actor then return end

        TotalTime = TotalTime + DeltaTime

        -- Print every second
        if math.floor(TotalTime) > math.floor(TotalTime - DeltaTime) then
            Print(string.format("[MovementTest] Tick working! TotalTime: %.2fs, DeltaTime: %.4fs", TotalTime, DeltaTime))
        end

        -- Try to move actor
        local Success, CurrentLocation = pcall(function() return Actor:GetActorLocation() end)

        if not Success then
            Print("[MovementTest] ERROR: GetActorLocation() failed!")
            return
        end

        if not CurrentLocation then
            Print("[MovementTest] ERROR: GetActorLocation() returned nil!")
            return
        end

        -- Move 100 units per second in X direction
        local MoveSpeed = 100.0
        local NewX = CurrentLocation.X + MoveSpeed * DeltaTime
        local NewLocation = FVector(NewX, CurrentLocation.Y, CurrentLocation.Z)

        Actor:SetActorLocation(NewLocation)

        -- Print location every 2 seconds
        if math.floor(TotalTime / 2.0) > math.floor((TotalTime - DeltaTime) / 2.0) then
            Print(string.format("[MovementTest] Location: (%.1f, %.1f, %.1f)", NewX, CurrentLocation.Y, CurrentLocation.Z))
        end
    end

    return ReturnTable
end
