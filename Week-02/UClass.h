#pragma once
#include "Containers.h"

class UClass
{
public:
	UClass(FString className, UClass* parentClass);
	UClass(FString className);
	~UClass();

	FString GetName() const { return ClassName; }
	UClass* GetParent() const { return ParentClass; }

private:
	FString ClassName;
	UClass* ParentClass;
};