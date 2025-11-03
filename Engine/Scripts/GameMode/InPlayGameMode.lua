
setmetatable(_ENV, { __index = EngineTypes })

local FVector = EngineTypes.FVector

-- ✅ FACTORY PATTERN - Safe for multiple actors sharing the same script!
-- Return a function that creates a new instance for each Actor
return function()
    -- Each instance gets its own variables (not shared!)
    local ReturnTable = {
        HasInitialized = false
        }

    function ReturnTable:BeginPlay()
        if self.HasInitialized then
            return
        end
        self.HasInitialized = true
        Print("[GameMode] InPlay Begin")
        self.GameMode = GetGameMode()        
        if not self.GameMode then
            Print("[GameMode] GameMode not found")
            return
        end
        self.GameMode:SpawnPlayerCharacter()
        Print("[GameMode] 123")
        self.GameMode:InitializeEnemyPool(10)
        Print("[GameMode] 456")
        self.GameMode:SpawnEnemies(1, FVector(1.0, 1.0, 0.0))
        Print("[GameMode] 789")
        local uuid = self.this.GetUUID and self.this:GetUUID() or "Unknown"
        local display_name = self.Name or (self.this.GetName and self.this:GetName() or "Actor")
        Print(string.format("[BeginPlay] %s (%s)", display_name, uuid))
        
        if self.this.PrintLocation then
            self.this:PrintLocation()
        end
    end

    function ReturnTable:Tick(DeltaTime)
        if not self.GameMode then
            return
        end
--     TODO GameMode Tick에서 돌릴 로직 추가
--       e.g.) 스코어 누적, 적 웨이브 증가 등등
    end

    return ReturnTable
end
