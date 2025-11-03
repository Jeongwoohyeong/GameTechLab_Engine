#include "pch.h"
#include "GameMode/Public/GameMode.h"

#include "Component/Public/PrimitiveComponent.h"
#include "Component/Public/ULuaScriptComponent.h"
#include "GamePlay/Public/PlayerController.h"
#include "Player/Public/PlayerCharacter.h"

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
    if (AActor* ControlledActor = GetPlayerController()->GetControlledActor())
    {
        if(ULuaScriptComponent* LuaComp = ControlledActor->GetLuaScriptComponent())
        {
            LuaComp->SetScriptName("Scripts/Player/PlayerCharacter.lua");
            if(LuaComp->LoadScript())
            {
                UE_LOG("[GameMode/SpawnPlayerCharacter] Lua script : %s load", LuaComp->GetScriptName().c_str());
                // LuaComp->ActivateFunction("OnBeginOverlap");
                // LuaComp->ActivateFunction("OnEndOverlap");
                // LuaComp->ActivateFunction("OnHit");
            }
            else
            {
                UE_LOG("[GameMode/SpawnPlayerCharacter] Load Fail %s", LuaComp->GetScriptName().c_str());
            }
        }
        else
        {
            UE_LOG_ERROR("[GameMode/SpawnPlayerCharacter] Lua script componenetn is null");
        }
    }
    else
    {
        UE_LOG_ERROR("[GameMode/SpawnPlayerCharacter] controlled actor is null");
    }
}

void AGameMode::InitializeEnemyPool(int32 EnemyCount)
{
    if (!OwningWorld)
    {
        UE_LOG_ERROR("[GameMode/InitializeEnemyPool] Cannot spawn enemy pawn: No owning world");
        return;
    }
    // Play 선택 후 코루틴으로 딜레이 주고 시작
    // 딜레이 사이에 object pool 준비
    Enemies.resize(EnemyCount);
    for (auto& Enemy : Enemies)
    {
        // 레벨에 적 추가
        APawn* EnemyPawn = Cast<APlayerCharacter>(OwningWorld->SpawnActor(APlayerCharacter::StaticClass()));
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
    UE_LOG("[GameMode/InitializeEnemyPool] Create object pool success");
}

void AGameMode::SpawnEnemies(int32 EnemyCount, FVector Location)
{    
    for (int i = 0; i < EnemyCount; i++)
    {
        if (auto Enemy = Enemies[i].Get())
        {
            Enemy->SetCanTick(true);
            Enemy->SetActorLocation(Location);
            
            for (auto& Component : Enemy->GetOwnedComponents())
            {
                if (UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(Component))
                {
                    PrimitiveComp->SetVisibility(true);
                    PrimitiveComp->SetCollisionEnabled(true);
                }
            }
        }
    }
}
