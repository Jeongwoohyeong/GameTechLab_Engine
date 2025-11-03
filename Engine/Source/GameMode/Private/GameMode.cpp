#include "pch.h"
#include "GameMode/Public/GameMode.h"

#include "Component/Public/PrimitiveComponent.h"
#include "Component/Public/ULuaScriptComponent.h"
#include "GamePlay/Public/PlayerController.h"
#include "Player/Public/PlayerCharacter.h"
#include "Player/Public/EnemyCharacter.h"

IMPLEMENT_CLASS(AGameMode, AGameModeBase)

AGameMode::AGameMode()
{
    bCanEverTick = true;
    DefaultPawnClass = APlayerCharacter::StaticClass();
    ScriptFilePath = {
        "Scripts/GameMode/MainMenuGameMode.lua",
        "Scripts/GameMode/InPlayGameMode.lua",
        "Scripts/GameMode/GameOverGameMode.lua"
    };    
}

AGameMode::~AGameMode()
{
}

void AGameMode::InitializeLuaScript()
{
    Super::InitializeLuaScript();
    this->SetUseScript(true);
    if (ULuaScriptComponent* LuaComponenent = this->GetLuaScriptComponent())
    {
        // TODO MainMenu 추가되면 InPlay 제거, MainMenu 활성화
        //LuaComponenent->SetScriptName(ScriptFilePath[static_cast<uint8>(CurrentState)]);
        LuaComponenent->SetScriptName(ScriptFilePath[static_cast<uint8>(EGameState::InPlay)]);
        if(LuaComponenent->LoadScript())
        {
            UE_LOG("[GameMode/InitializeLuaScript] Lua script : %s load", LuaComponenent->GetScriptName().c_str());            
        }
        else
        {
            UE_LOG_ERROR("[GameMode/InitializeLuaScript] Load Fail %s", LuaComponenent->GetScriptName().c_str());
        }
    }
    else
    {
        UE_LOG_ERROR("[GameMode/InitializeLuaScript] Lua script componenetn is null");
    }
}

void AGameMode::InitGame()
{
    AGameModeBase::InitGame();
    UE_LOG("[GameMode] InitGameMode");
}

void AGameMode::StartPlay()
{
    AGameModeBase::StartPlay();    
}

void AGameMode::BeginPlay()
{
    AGameModeBase::BeginPlay();
    StartPlay();
    if (GetLuaScriptComponent())
    {
        GetLuaScriptComponent()->ActivateFunction("BeginPlay");
    }
}

void AGameMode::Tick(float DeltaTime)
{
    AGameModeBase::Tick(DeltaTime);
    // 현재 lua script tick에서 처리하는 게 없어서 주석처리
//GetLuaScriptComponent()->ActivateFunction("Tick");
}

void AGameMode::ChangeState(EGameState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }
    
    ULuaScriptComponent* LuaComponenent = this->GetLuaScriptComponent();
    
    if (!LuaComponenent)
    {
        UE_LOG_ERROR("[GameMode] LuaComponent not found");
        return;
    }
    
    switch (NewState)
    {
    case EGameState::MainMenu:
        LuaComponenent->SetScriptName(ScriptFilePath[static_cast<uint8>(EGameState::MainMenu)]);
        break;
    case EGameState::InPlay:
        LuaComponenent->SetScriptName(ScriptFilePath[static_cast<uint8>(EGameState::InPlay)]);
        break;
    case EGameState::GameOver:
        LuaComponenent->SetScriptName(ScriptFilePath[static_cast<uint8>(EGameState::GameOver)]);
        break;
    default:
        LuaComponenent->SetScriptName(ScriptFilePath[static_cast<uint8>(EGameState::MainMenu)]);
        break;        
    }

    if(LuaComponenent->LoadScript())
    {        
        UE_LOG("[GameMode/ChangeState] Lua script : %s load", LuaComponenent->GetScriptName().c_str());        
    }
    else
    {
        UE_LOG("[GameMode/ChangeState] Load Fail %s", LuaComponenent->GetScriptName().c_str());
    }
}

void AGameMode::SpawnPlayerCharacter()
{
    InitializePlayerController();    
}

void AGameMode::InitializeEnemyPool(int32 EnemyCount)
{
    if (!OwningWorld)
    {
        UE_LOG_ERROR("[GameMode/InitializeEnemyPool] Cannot spawn enemy pawn: No owning world");
        return;
    }

    // 기존 적들 정리 (중복 생성 방지)
    if (!Enemies.empty())
    {
        UE_LOG("[GameMode/InitializeEnemyPool] Clearing existing enemy pool (%zu enemies)", Enemies.size());
        for (auto& EnemyPtr : Enemies)
        {
            if (APawn* Enemy = EnemyPtr.Get())
            {
                OwningWorld->DestroyActor(Enemy);
            }
        }
        Enemies.clear();
    }

    // Play 선택 후 코루틴으로 딜레이 주고 시작
    // 딜레이 사이에 object pool 준비
    Enemies.resize(EnemyCount);
    for (auto& Enemy : Enemies)
    {
        // 레벨에 적 추가 (AEnemyCharacter 사용)
        APawn* EnemyPawn = Cast<AEnemyCharacter>(OwningWorld->SpawnActor(AEnemyCharacter::StaticClass()));
        EnemyPawn->SetCanTick(false);
        for (auto& Component : EnemyPawn->GetOwnedComponents())
        {
            if (UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(Component))
            {
                PrimitiveComp->SetVisibility(false);
                PrimitiveComp->SetCollisionEnabled(false);
            }
        }
        Enemy = TWeakObjectPtr<APawn>(EnemyPawn);
    }
    UE_LOG("[GameMode/InitializeEnemyPool] Create object pool success (Created %d enemies)", EnemyCount);
}

void AGameMode::SpawnEnemies(int32 EnemyCount, FVector Location)
{    
    if (EnemyCount <= 0)
    {
        return;
    }

    int32 Spawned = 0;
    for (int i = CurrentEnemyIndex; i < Enemies.size(); i++)
    {
        if (Spawned >= EnemyCount)
        {
            break;
        }
        if (auto Enemy = Enemies[i].Get())
        {
            if (!Enemy->CanTick())
            {
                Enemy->SetActorLocation(Location);
                Enemy->SetCanTick(true);

                for (auto& Comp : Enemy->GetOwnedComponents())
                {
                    if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
                    {
                        Prim->SetCollisionEnabled(true);
                        Prim->SetVisibility(true);
                    }
                }
                Spawned++;
            }
        }
    }

    CurrentEnemyIndex = std::min(CurrentEnemyIndex + EnemyCount, MaxPoolSize);

    UE_LOG("[GameMode/SpawnEnemies] Spawned %d enemies current wave", Spawned);
}
