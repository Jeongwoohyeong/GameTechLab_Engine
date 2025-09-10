#pragma once
#include "UClass.h"

//#ifdef _UNICODE
//#define TEXT(str) L##str
//#else
//#define TEXT(str) str
//#endif // _UNICODE

#define RTTI_ROOT_DECLARE()																\
	public:																				\
		static UClass* StaticClass();													\
		virtual UClass* GetClass() const;												\

#define RTTI_DECLARE()																	\
	public:																				\
		static UClass* StaticClass();													\
		virtual UClass* GetClass() const;												\

#define RTTI_IMPL(className, parentClass)												\
	UClass* className::StaticClass()													\
	{																					\
		static UClass ClassInfo = UClass(#className, parentClass::StaticClass());		\
		return &ClassInfo;																\
	}																					\
	UClass* className::GetClass() const													\
	{																					\
		return className::StaticClass();												\
	}																					\

#define RTTI_IMPL_ROOT(className)														\
	UClass* className::StaticClass()													\
	{																					\
		static UClass ClassInfo = UClass(#className);									\
		return &ClassInfo;																\
	}																					\
	UClass* className::GetClass() const													\
	{																					\
		return className::StaticClass();												\
	}																					\
