
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
        MaxPoolSize = 30,
        WaveDelay = 5.0,
        BroadCastDelay = 3.0
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

        -- PIE 모드 시작 시에는 래터박스를 켜지 않음
        Print("[GameMode] Press 'Start Game' to begin playing")
    end

    function ReturnTable:StartGame()
        if not self.GameMode then
            return
        end

        Print("[GameMode] START GAME!")

        -- 게임 시작 시 레터박스 활성화 후 3초 뒤에 페이드아웃
        local PlayerController = self.GameMode:GetPlayerController()
        if PlayerController then
            local camMgr = PlayerController:GetPlayerCameraManager()
            if camMgr then
                -- 즉시 레터박스 켜기 (높이 10%, 블렌드 시간 0.5초)
                camMgr:StartLetterBox(0.1, 0.5)
                Print("[GameMode] Letter box activated on game start")
            end
        end

        -- 3초 후 레터박스 페이드아웃 시작 (코루틴 사용)
        StartCoroutine(self, "LetterboxFadeOut")

        -- 적 풀 초기화 (Start Game 버튼 누를 때만 생성)
        if not self.GameStarted then
            Print("[GameMode] Initializing enemy pool...")
            self.GameMode:InitializeEnemyPool(10)
            Print("[GameMode] Enemy pool initialized")
            self.GameStarted = true
        end

        StartCoroutine(self, "WaveSpawner")
--         StartCoroutine(self, "BroadCastPlayerPosition")

        -- 플레이어 무기 활성화
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

    -- 레터박스 페이드아웃 코루틴
    function ReturnTable:LetterboxFadeOut()
        -- 5초 대기
        wait(5.0)

        local PlayerController = self.GameMode:GetPlayerController()
        if PlayerController then
            local camMgr = PlayerController:GetPlayerCameraManager()
            if camMgr then
                -- 2초에 걸쳐 레터박스 사라지게
                camMgr:StopLetterBox(2.0)
                Print("[GameMode] Letter box fading out after 5 seconds")
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
                local RandomX = (math.random() - 0.5) * 1000.0
                local RandomY = (math.random() - 0.5) * 1000.0
                local RandomZ = (math.random() - 0.5) * 1000.0
                local SpawnPos = FVector(RandomX, RandomY, RandomZ)
                self.GameMode:SpawnEnemies(1, SpawnPos)
            end
        
            TotalSpawned = TotalSpawned + SpawnCount
            Print(string.format("[GameMode] Total spawned enemies %d", TotalSpawned))
            self.CurrentWave = self.CurrentWave + 1
            
            wait(self.WaveDelay)
        end
    end

--     function ReturnTable:BroadCastPlayerPosition()
--         while true do            
--             local PlayerController = self.GameMode:GetPlayerController()
--             if not PlayerController then
--                 Print("[GameMode] PlayerController not found") 
--                 break           
--             end
--             self.GameMode:BroadCastPlayerLocation()
--             wait(self.BroadCastDelay)
--         end
--     end
            
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