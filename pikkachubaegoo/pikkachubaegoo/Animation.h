#pragma once
#include <string>
#include <map>
#include <vector>

/*
	현재 상태에 시간 변화에 따른 애님 변화
	상태 변화 시에도 애니매이션 변화
	상태 변화 함수
*/

class UAnimation
{
public:
	UAnimation();
	~UAnimation();

	const std::vector<std::string>* GetClipList(const std::string& name) const;

private:
	std::map<std::string, std::vector<std::string>> clipName;
};