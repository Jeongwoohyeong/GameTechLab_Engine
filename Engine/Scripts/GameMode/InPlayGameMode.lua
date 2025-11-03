
setmetatable(_ENV, { __index = EngineTypes })

local FVector = EngineTypes.FVector

-- ✅ FACTORY PATTERN - Safe for multiple actors sharing the same script!
-- Return a function that creates a new instance for each Actor
return function()
    Print("qqqqqqqqqqqppppp")
    -- Each instance gets its own variables (not shared!)
    local ReturnTable = {
        HasInitialized = false,
        GameStarted = false,
        CurrentWave = 1,
        MaxPoolSize = 10,
        WaveDelay = 5.0
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

        -- 플레이어 컨트롤러가 이미 존재하는지 체크
        local PlayerController = self.GameMode:GetPlayerController()
        if not PlayerController then
            Print("[GameMode] No existing player controller, spawning new one")
            self.GameMode:SpawnPlayerCharacter()
        else
            Print("[GameMode] Player controller already exists, skipping spawn")
        end

        -- 게임 시작은 Start Game 버튼을 눌러야만!
        Print("[GameMode] Press 'Start Game' to begin playing")
    end

    function ReturnTable:StartGame()
        if not self.GameMode then
            return
        end

        Print("[GameMode] START GAME!")

        -- 적 풀 초기화 (Start Game 버튼 누를 때만 생성)
        if not self.GameStarted then
            Print("[GameMode] Initializing enemy pool...")
            self.GameMode:InitializeEnemyPool(10)
            Print("[GameMode] Enemy pool initialized")
            self.GameStarted = true
        end
    
        StartCoroutine(self, "WaveSpawner")

        -- 플레이어 무기 활성화
        local PlayerController = self.GameMode:GetPlayerController()
        if PlayerController then
            local PlayerPawn = PlayerController:GetControlledPawn()
            if PlayerPawn then
                local LuaComp = PlayerPawn:GetLuaScriptComponent()
                if LuaComp then
                    LuaComp:ActivateFunction("EnableWeapon")
                    Print("[GameMode] Player weapon enabled")
                end
            end
        end
    end

    function ReturnTable:WaveSpawner()
        local TotalSpawned = 0
        while TotalSpawned < self.MaxPoolSize do
            local SpawnCount = math.min(self.CurrentWave * 2, self.MaxPoolSize - TotalSpawned)
            if SpawnCount <= 0 then
                Print("[GameMode] Enemy pool limit reached.")
                break
            end
                                    
            Print(string.format("[GameMode] Wave %d spawned %d enemies", self.CurrentWave, SpawnCount))
            for i = 1, SpawnCount do
                local RandomX = (math.random() - 0.5) * 400.0
                local RandomY = (math.random() - 0.5) * 400.0
                local BaseZ = 0.0
                local SpawnPos = FVector(RandomX, RandomY, BaseZ)
                self.GameMode:SpawnEnemies(1, SpawnPos)
            end
        
            TotalSpawned = TotalSpawned + SpawnCount
            Print(string.format("[GameMode] Total spawned enemies %d", TotalSpawned))
            self.CurrentWave = self.CurrentWave + 1
            
            wait(self.WaveDelay)
        end
    end

    function ReturnTable:Tick(DeltaTime)
        if not self.GameMode then
            return
        end
--         Print("Lua Tick");

--     TODO GameMode Tick에서 돌릴 로직 추가
--       e.g.) 스코어 누적, 적 웨이브 증가 등등
    end

    return ReturnTable
end
