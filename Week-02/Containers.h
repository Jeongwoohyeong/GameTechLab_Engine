#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>

////////////////////////////////////////////////////
//
//	
//
//		래핑 모음
//
//
//
//
////////////////////////////////////////////////////

////////////////////////////////////////////////////
//
//			STL 컨테이너
//
////////////////////////////////////////////////////

template<typename T>
using TArray = std::vector<T>;

template<typename V, typename K>
using TMap = std::unordered_map<V, K>;

template<typename T>
using TSet = std::unordered_set<T>;

template<typename T1, typename T2>
using TPair = std::pair<T1, T2>;