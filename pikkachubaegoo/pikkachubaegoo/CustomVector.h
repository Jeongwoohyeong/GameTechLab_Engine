#pragma once
#include <iostream>

using namespace std;

template<typename T>
class UCustomVector
{
private:
	T* DynamicArray;
	uint32_t DataCount;
	uint32_t ArrayCount;
	bool ArrayExpansion() 
	{
		if (ArrayCount >= 0x80000000)
		{
			//크기제한
			cout << "vector size error" << endl;
			return false;
		}
		uint32_t newArrayCount = ArrayCount << 1;
		T* newDynamicArray = new T[newArrayCount];
		for (int i = 0; i < DataCount; i++)
		{
			newDynamicArray[i] = DynamicArray[i];
		}
		delete[] DynamicArray;
		DynamicArray = newDynamicArray;
		ArrayCount = newArrayCount;
		return true;
	}
public:
	UCustomVector<T>()
	{
		DynamicArray = new T[1];
		DataCount = 0;
		ArrayCount = 1;
	}
	UCustomVector<T>(int size)
	{
		DynamicArray = new T[size];
		DataCount = 0;
		ArrayCount = size;
	}
	~UCustomVector()
	{
		delete[] DynamicArray;
	}
	
	void PushBack(const T& element) 
	{
		if (DataCount == ArrayCount)
		{
			bool bExpansionSuccess = ArrayExpansion();
			if (bExpansionSuccess == false)
			{
				return;
			}
		}
		DynamicArray[DataCount] = element;
		DataCount++;
	}
	void PushBack(T& element)
	{
		if (DataCount == ArrayCount)
		{
			bool bExpansionSuccess = ArrayExpansion();
			if (bExpansionSuccess == false)
			{
				return;
			}
		}
		DynamicArray[DataCount] = element;
		DataCount++;
	}
	void PopBack()
	{
		DynamicArray[DataCount] = nullptr;
		DataCount--;
	}
	T& operator [](uint32_t idx)
	{
		return DynamicArray[idx];
	}
	const uint32_t Size() const
	{
		return DataCount;
	}
	void Clear()
	{
		for (int i = 0; i < DataCount; i++) 
		{
			DynamicArray[i] = nullptr;
		}
		DataCount = 0;
	}
	//push
	//pop
	//insert
	//eraser
	//size
	
};