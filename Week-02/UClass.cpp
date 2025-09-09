#include "UClass.h"

UClass::UClass(FString className, UClass* parentClass)
	:ClassName(className), ParentClass(parentClass)
{
}

UClass::UClass(FString className)
	:ClassName(className), ParentClass(nullptr)
{
}

UClass::~UClass()
{
}