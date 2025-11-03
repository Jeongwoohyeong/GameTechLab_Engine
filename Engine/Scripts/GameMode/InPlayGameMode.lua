
setmetatable(_ENV, { __index = EngineTypes })

local FVector = EngineTypes.FVector

-- ✅ FACTORY PATTERN - Safe for multiple actors sharing the same script!
-- Return a function that creates a new instance for each Actor
return function()
    Print("qqqqqqqqqqqppppp")
    -- Each instance gets its own variables (not shared!)
    local ReturnTable = {
        HasInitialized = false,
        GameStarted = false
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

        -- 적 스폰
        self.GameMode:SpawnEnemies(1, FVector(1.0, 1.0, 0.0))

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

    function ReturnTable:Tick(DeltaTime)
        if not self.GameMode then
            return
        end

--     TODO GameMode Tick에서 돌릴 로직 추가
--       e.g.) 스코어 누적, 적 웨이브 증가 등등
    end

    return ReturnTable
end
