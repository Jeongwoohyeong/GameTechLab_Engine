setmetatable(_ENV, { __index = EngineTypes })

local FVector = EngineTypes.FVector

return function()
    local ReturnTable = {}
    Print("ppppp")

    function ReturnTable:OnBeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult)
        Print("[Lua/Player] Begin overlap")
    end

    function ReturnTable:OnEndOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex)
        Print("[Lua/Player] End overlap")
    end

    function ReturnTable:OnHit(OverlappedComp, OtherActor, OtherComp, NormalImpulse, OutHit)
        Print("[Lua/Player] Hit")
    end

    return ReturnTable
end