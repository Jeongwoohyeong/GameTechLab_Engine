#pragma once
#include "GameMode/Public/GameModeBase.h"

class APlayerCharacter;

UCLASS()
class AGameMode : public AGameModeBase
{
    GENERATED_BODY()
    DECLARE_CLASS(AGameMode, AGameModeBase)
public:
    enum class EGameState : uint8
    {
        MainMenu,
        InPlay,
        GameOver
    };
public:
    AGameMode();
    ~AGameMode() override;

    void InitializeLuaScript() override;

    void InitGame() override;
    void StartPlay() override;
    void BeginPlay() override;
    void Tick(float DeltaTime) override;

    // MainMenu, InPlay, GameOver 상태 변경 - Lua에서 제어
    void ChangeState(EGameState NewState);
    // MainMenu에서 InPlay로 넘어올 때 코루틴으로 딜레이주고 호출
    void SpawnPlayerCharacter();
    void InitializeEnemyPool(int32 EnemyCount);

    // InPlay에서 코루틴으로 생성되는 양 조절
    void SpawnEnemies(int32 EnemyCount, FVector Location);

    APlayerController* GetPlayerController() const override { return Super::GetPlayerController(); }
private:
    EGameState CurrentState = EGameState::MainMenu;
    int32 CurrentEnemyIndex = 0;
    int32 MaxPoolSize = 0;
};