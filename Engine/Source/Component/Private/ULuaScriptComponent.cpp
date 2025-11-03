#include "pch.h"
#include "Component/Public/ULuaScriptComponent.h"
#include "Manager/Lua/Public/LuaScriptManager.h"
#include "Manager/Coroutine/Public/LuaCoroutineManager.h"
#include "Actor/Public/Actor.h"
#include "Utility/Public/JsonSerializer.h"

IMPLEMENT_CLASS(ULuaScriptComponent, UActorComponent)

ULuaScriptComponent::ULuaScriptComponent()
{
    bCanEverTick = true;
}

ULuaScriptComponent::~ULuaScriptComponent()
{
}

void ULuaScriptComponent::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
    Super::Serialize(bInIsLoading, InOutHandle);
    
    if (bInIsLoading)
    {
        FJsonSerializer::ReadString(InOutHandle, "ScriptName", ScriptName, "");
    }
    else
    {
        InOutHandle["ScriptName"] = ScriptName;
    }
}

UObject* ULuaScriptComponent::Duplicate()
{
    ULuaScriptComponent* NewComponent = Cast<ULuaScriptComponent>(Super::Duplicate());
    if (!NewComponent)
        return nullptr;
    
    NewComponent->ScriptName = ScriptName;
    NewComponent->SelfTable = SelfTable;
    
    return NewComponent;
}

void ULuaScriptComponent::DuplicateSubObjects(UObject* DuplicatedObject)
{
    Super::DuplicateSubObjects(DuplicatedObject);
}

void ULuaScriptComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Safety check: Ensure we have a valid owner
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        std::cerr << "[ERROR] ULuaScriptComponent::BeginPlay: No owner!" << std::endl;
        return;
    }

    // 스크립트가 아직 로드되지 않았으면 로드 (SetScriptName에서 이미 로드했을 수 있음)
    if (!SelfTable.valid())
    {
        // If ScriptName is empty, skip loading - it will be set later via SetScriptName
        if (ScriptName.empty())
        {
            UE_LOG("[ULuaScriptComponent] BeginPlay: ScriptName is empty, skipping load (will be set later)");
            return;
        }

        // Load script first
        if (!LoadScript())
        {
            std::cerr << "[ERROR] ULuaScriptComponent::BeginPlay: Failed to load script: " << ScriptName << std::endl;
            return;
        }
    }
    else
    {
        UE_LOG("[ULuaScriptComponent] BeginPlay: Script already loaded: '%s'", ScriptName.c_str());
    }

    // Register with LuaScriptManager for hot-reload tracking
    FLuaScriptManager::GetInstance().RegisterComponent(this);

    if (SelfTable.valid())
    {
        UE_LOG("[ULuaScriptComponent] BeginPlay: SelfTable is valid");
        if (SelfTable["BeginPlay"].valid())
        {
            UE_LOG("[ULuaScriptComponent] BeginPlay: Calling Lua BeginPlay()");
            ActivateFunction("BeginPlay");
        }
        else
        {
            UE_LOG_ERROR("[ULuaScriptComponent] BeginPlay: Lua BeginPlay() function not found in table!");
        }
    }
    else
    {
        UE_LOG_ERROR("[ULuaScriptComponent] BeginPlay: SelfTable is invalid!");
    }
}

void ULuaScriptComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    // Safety check: Only tick if we have a valid owner
    AActor* Owner = GetOwner();
    if (!Owner || !SelfTable.valid() || !SelfTable["Tick"].valid())
    {
        return;
    }

    // Additional safety: Only tick if the actor has begun play
    // This prevents ticking during shutdown
    if (!Owner->HasBegunPlay())
    {
        return;
    }

    ActivateFunction("Tick", DeltaTime);
}

void ULuaScriptComponent::EndPlay()
{
    Super::EndPlay();

    // FIRST: Invalidate the actor reference in Lua to prevent access
    if (SelfTable.valid())
    {
        SelfTable["this"] = sol::nil;  // ✅ 먼저 Actor 포인터 무효화!

        if (SelfTable["EndPlay"].valid())
        {
            ActivateFunction("EndPlay");
        }
    }

    // Stop all coroutines started by this component
    for (int coroutineID : ActiveCoroutineIDs)
    {
        std::cout << "[Component] Stopping coroutine " << coroutineID << " (EndPlay)" << std::endl;
        FLuaCoroutineManager::GetInstance().StopCoroutine(coroutineID);
    }
    ActiveCoroutineIDs.clear();

    // Unregister from LuaScriptManager
    FLuaScriptManager::GetInstance().UnregisterComponent(this);

    // Invalidate the Lua table to prevent any further access
    SelfTable = sol::nil;
}

void ULuaScriptComponent::SetScriptName(const FString& InScriptName)
{
    // Scripts/ 경로 추가 (필요한 경우)
    if (!InScriptName.empty() && InScriptName.find("Scripts/") == std::string::npos)
    {
        ScriptName = "Scripts/" + InScriptName + ".lua";
    }
    else
    {
        ScriptName = InScriptName;
    }

    UE_LOG("[ULuaScriptComponent] SetScriptName: '%s'", ScriptName.c_str());

    // 스크립트 즉시 로드
    if (!LoadScript())
    {
        UE_LOG_ERROR("[ULuaScriptComponent] SetScriptName: Failed to load script");
        return;
    }

    // Register with LuaScriptManager for hot-reload tracking
    FLuaScriptManager::GetInstance().RegisterComponent(this);

    // If the owner has already begun play, call Lua BeginPlay now
    AActor* Owner = GetOwner();
    if (Owner && Owner->HasBegunPlay() && SelfTable.valid())
    {
        if (SelfTable["BeginPlay"].valid())
        {
            UE_LOG("[ULuaScriptComponent] SetScriptName: Calling Lua BeginPlay()");
            ActivateFunction("BeginPlay");
        }
        else
        {
            UE_LOG_ERROR("[ULuaScriptComponent] SetScriptName: Lua BeginPlay() function not found in table!");
        }
    }
}

bool ULuaScriptComponent::LoadScript()
{
    if (ScriptName.empty())
    {
        FString ClassName = GetOwner() ? GetOwner()->GetClass()->GetName().ToString() : FString("Actor");
        ScriptName = "Scripts/" + ClassName + ".lua";
    }

    UE_LOG("[ULuaScriptComponent] LoadScript: Attempting to load '%s'", ScriptName.c_str());

    // Load from LuaScriptManager
    SelfTable = FLuaScriptManager::GetInstance().CreateLuaTable(ScriptName);

    if (!SelfTable.valid())
    {
        UE_LOG_ERROR("[ULuaScriptComponent] LoadScript: Failed to load script '%s'", ScriptName.c_str());
        return false;
    }

    UE_LOG("[ULuaScriptComponent] LoadScript: Successfully loaded '%s'", ScriptName.c_str());

    // Bind self.this to the owning Actor
    AActor* Owner = GetOwner();
    if (Owner)
    {
        SelfTable["this"] = Owner;
        SelfTable["Name"] = Owner->GetName().ToString();
    }

    return true;
}

void ULuaScriptComponent::ActivateFunctionLua(const FString& FunctionName, sol::variadic_args va)
{
    if (!SelfTable.valid())
    {
        UE_LOG_ERROR("[ULuaScriptComponent/ActivateFunctionLua] SelfTable invalid");
        return;
    }

    sol::function LuaFunction = SelfTable[FunctionName.c_str()];
    if (!LuaFunction.valid())
    {
        UE_LOG_ERROR("[ULuaScriptComponent/ActivateFunctionLua] LuaFunction %s invalid", FunctionName.c_str());
        return;
    }
    LuaFunction(SelfTable, sol::as_args(va));
}

void ULuaScriptComponent::RegisterCoroutine(int coroutineID)
{
    ActiveCoroutineIDs.push_back(coroutineID);
}
