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
    bool IsScriptLoaded() const { return SelfTable.valid(); }

    template<typename... Args>
    void ActivateFunction(const FString& FunctionName, Args&&... args);

    // 가변인자 헬퍼 함수
    void ActivateFunctionLua(const FString& FunctionName, sol::variadic_args va);

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
    // FString을 std::string으로 명시적 변환 (Release 빌드 안정성)
    std::string funcName = FunctionName.c_str();

    if (SelfTable.valid())
    {
        // sol::table 인덱스 접근을 std::string으로
        sol::object funcObj = SelfTable[funcName];
        if (funcObj.valid() && funcObj.is<sol::function>())
        {
            //UE_LOG("[ULuaScriptComponent] ActivateFunction: Calling '%s'", funcName.c_str());
            sol::function func = funcObj;
            auto Result = func(SelfTable, std::forward<Args>(args)...);
            if (!Result.valid())
            {
                sol::error err = Result;
                UE_LOG_ERROR("[ULuaScriptComponent] Lua function '%s' failed: %s", funcName.c_str(), err.what());
            }
            else
            {
                //UE_LOG("[ULuaScriptComponent] ActivateFunction: '%s' executed successfully", funcName.c_str());
            }
        }
        else
        {
            UE_LOG_ERROR("[ULuaScriptComponent] ActivateFunction: Function '%s' not valid in table", funcName.c_str());
        }
    }
    else
    {
        UE_LOG_ERROR("[ULuaScriptComponent] ActivateFunction: SelfTable is not valid");
    }
}
