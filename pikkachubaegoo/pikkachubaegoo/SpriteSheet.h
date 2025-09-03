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

private:
	FSpriteMeta metaData;
	std::string spriteFramePath;
	std::map<std::string, FSpriteFrame> frameDatas;
	void* texture;
};