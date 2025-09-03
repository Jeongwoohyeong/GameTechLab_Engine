#pragma once
#include "SpriteStruct.h"
#include <map>

class USpriteSheet
{
public:
	USpriteSheet();
	~USpriteSheet();	
	bool Load(std::string jsonPath);
	const FSpriteFrame* GetFrame(const std::string& spriteName) const;
	const FSpriteMeta* GetMetaData() const { return &metaData; }
	const std::wstring GetImagePath() const { return spriteImagePath; }
	FVector4 GetSourceRect(const std::string& spriteName);

private:
	std::wstring ConvertStringToWstring(std::string&);

private:
	FSpriteMeta metaData;

	std::wstring spriteImagePath;
	std::string spriteFramePath;
	std::map<std::string, FSpriteFrame> frameDatas;
};