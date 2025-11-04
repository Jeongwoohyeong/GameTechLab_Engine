-- ActorTimeDilationExample.lua
-- This script demonstrates Actor-specific time dilation (CustomTimeDilation)
-- Each actor with this script can have its own independent time scale
-- while other actors continue at normal speed

setmetatable(_ENV, { __index = EngineTypes })

return function()
    local ReturnTable = {}
    local Actor = nil
    local InitialTimeDilation = 0.5  -- Start at half speed

    -- Called when the actor begins play
    function ReturnTable:BeginPlay()
        Actor = self.this

        -- Set this actor to run at custom speed (independent of global time scale)
        Actor:SetCustomTimeDilation(InitialTimeDilation)

        Print(string.format("[ActorTimeDilation] %s initialized with %.2fx speed",
            Actor:GetName(), Actor:GetCustomTimeDilation()))
        Print("[ActorTimeDilation] Press keys to control THIS actor's speed:")
        Print("[ActorTimeDilation]   Q = Increase speed (+0.1x)")
        Print("[ActorTimeDilation]   E = Decrease speed (-0.1x)")
        Print("[ActorTimeDilation]   R = Reset to normal speed (1.0x)")
        Print("[ActorTimeDilation]   F = Set to very slow (0.2x)")
        Print("[ActorTimeDilation]   G = Set to very fast (3.0x)")
    end

    -- Called every frame
    function ReturnTable:Tick(DeltaTime)
        if not Actor then return end

        local InputManager = GetInputManager()

        -- Key Q: Increase speed
        if InputManager:IsKeyPressed("Key_Q") then
            local NewSpeed = Actor:GetCustomTimeDilation() + 0.1
            Actor:SetCustomTimeDilation(NewSpeed)
            Print(string.format("[ActorTimeDilation] Speed increased to %.2fx", Actor:GetCustomTimeDilation()))
        end

        -- Key E: Decrease speed
        if InputManager:IsKeyPressed("Key_E") then
            local NewSpeed = Actor:GetCustomTimeDilation() - 0.1
            Actor:SetCustomTimeDilation(NewSpeed)
            Print(string.format("[ActorTimeDilation] Speed decreased to %.2fx", Actor:GetCustomTimeDilation()))
        end

        -- Key R: Reset to normal speed
        if InputManager:IsKeyPressed("Key_R") then
            Actor:SetCustomTimeDilation(1.0)
            Print(string.format("[ActorTimeDilation] Speed reset to 1.0x"))
        end

        -- Key F: Set to very slow
        if InputManager:IsKeyPressed("Key_F") then
            Actor:SetCustomTimeDilation(0.2)
            Print(string.format("[ActorTimeDilation] Speed set to 0.2x (very slow)"))
        end

        -- Key G: Set to very fast
        if InputManager:IsKeyPressed("Key_G") then
            Actor:SetCustomTimeDilation(3.0)
            Print(string.format("[ActorTimeDilation] Speed set to 3.0x (very fast)"))
        end

        -- Key C: Display current state
        if InputManager:IsKeyPressed("Key_C") then
            local CurrentSpeed = Actor:GetCustomTimeDilation()
            local TimeManager = GetTimeManager()
            local GlobalDilation = TimeManager:GetGlobalTimeDilation()
            local EffectiveSpeed = CurrentSpeed * GlobalDilation

            Print(string.format("[ActorTimeDilation] %s State:", Actor:GetName()))
            Print(string.format("  Custom Time Dilation: %.2fx", CurrentSpeed))
            Print(string.format("  Global Time Dilation: %.2fx", GlobalDilation))
            Print(string.format("  Effective Speed: %.2fx", EffectiveSpeed))
        end

        -- Move actor in a circular pattern to demonstrate time effect
        -- The movement speed is affected by CustomTimeDilation
        local CurrentLocation = Actor.ActorLocation
        local MoveSpeed = 100.0  -- Units per second
        local CircleRadius = 200.0
        local AngularSpeed = 0.5  -- Radians per second

        -- Calculate circular motion
        local GameTime = GetTimeManager():GetGameTime()
        local Angle = GameTime * AngularSpeed
        local TargetX = CurrentLocation.x + MoveSpeed * DeltaTime
        local TargetY = CurrentLocation.y + math.sin(Angle) * CircleRadius * DeltaTime
        local TargetZ = CurrentLocation.z

        Actor.ActorLocation = FVector(TargetX, TargetY, TargetZ)

        -- Note: The actual DeltaTime passed to this function is already multiplied by:
        -- 1. Global Time Dilation (from TimeManager)
        -- 2. Custom Time Dilation (from Actor)
        -- So the movement automatically respects both time scales!
    end

    return ReturnTable
end
