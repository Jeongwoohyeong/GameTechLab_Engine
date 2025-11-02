#include "pch.h"
#include "GameMode/Public/GameMode.h"

#include "Component/Public/PrimitiveComponent.h"
#include "Player/Public/PlayerCharacter.h"

IMPLEMENT_CLASS(AGameMode, AGameModeBase)

AGameMode::AGameMode()
{
    bCanEverTick = true;
    DefaultPawnClass = APlayerCharacter::StaticClass();
}

AGameMode::~AGameMode()
{
}

void AGameMode::InitGame()
{
    AGameModeBase::InitGame();
    UE_LOG("[GameMode] InitGameMode");
}

void AGameMode::StartPlay()
{
    AGameModeBase::StartPlay();
    ChangeState(EGameState::MainMenu);
}

void AGameMode::BeginPlay()
{
    AGameModeBase::BeginPlay();
}

void AGameMode::Tick(float DeltaTime)
{
    AGameModeBase::Tick(DeltaTime);
}

void AGameMode::ChangeState(EGameState NewState)
{
    switch (NewState)
    {
    case EGameState::MainMenu:
        // TODO state 변경 시 Lua script 변경
        break;
    case EGameState::Play:
        // TODO state 변경 시 Lua script 변경
        break;
    case EGameState::End:
        // TODO state 변경 시 Lua script 변경
        break;
    default:
        // MainMenu 스크립트
        break;        
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
        UE_LOG_ERROR("[GameMode] Cannot spawn enemy pawn: No owning world");
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
}

void AGameMode::SpawnEnemies(int32 EnemyCount, FVector Location)
{    
    for (int i = 0; i < EnemyCount; i++)
    {
        if (auto Enemy = Enemies[i].Get())
        {
            for (auto& Component : Enemy->GetOwnedComponents())
            {
                if (UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(Component))
                {
                    PrimitiveComp->SetVisibility(true);
                    PrimitiveComp->SetCollisionEnabled(true);
                }
            }
            Enemy->SetCanTick(true);
            Enemy->SetActorLocation(Location);
        }
    }
}
