#pragma once
#include "sol/sol.hpp"
#include "Global/Types.h"
#include "Component/Public/ActorComponent.h"
#include <string>

// Forward declaration
class AActor;

UCLASS()
class ULuaScriptComponent : public UActorComponent
{
    GENERATED_BODY()
    DECLARE_CLASS(ULuaScriptComponent, UActorComponent)
    
public:
    ULuaScriptComponent();
    ~ULuaScriptComponent() override;

    // Serialize
    virtual void Serialize(const bool bInIsLoading, JSON& InOutHandle) override;

    // Duplicate
    virtual UObject* Duplicate() override;
    virtual void DuplicateSubObjects(UObject* DuplicatedObject) override;

    // Lifecycle functions
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual void EndPlay() override;
    
    // Script management
    FString GetScriptName() const { return ScriptName; }
    bool LoadScript();
    
    template<typename... Args>
    void ActivateFunction(const FString& FunctionName, Args&&... args);
    
    sol::table& GetLuaSelfTable() { return SelfTable; }
    void SetScriptName(const FString& InScriptName);

private:
    FString ScriptName;
    sol::table SelfTable;
    TArray<int> ActiveCoroutineIDs;  // Track coroutines started by this component

public:
    void RegisterCoroutine(int coroutineID);
};

template<typename ...Args>
inline void ULuaScriptComponent::ActivateFunction(const FString& FunctionName, Args && ...args)
{
    if (SelfTable.valid() && SelfTable[FunctionName].valid())
    {
        //UE_LOG("[ULuaScriptComponent] ActivateFunction: Calling '%s'", FunctionName.c_str());
        auto Result = SelfTable[FunctionName](SelfTable, std::forward<Args>(args)...);
        if (!Result.valid())
        {
            sol::error err = Result;
            UE_LOG_ERROR("[ULuaScriptComponent] Lua function '%s' failed: %s", FunctionName.c_str(), err.what());
        }
        else
        {
            //UE_LOG("[ULuaScriptComponent] ActivateFunction: '%s' executed successfully", FunctionName.c_str());
        }
    }
    else
    {
        UE_LOG_ERROR("[ULuaScriptComponent] ActivateFunction: Function '%s' not valid in table", FunctionName.c_str());
    }
}
