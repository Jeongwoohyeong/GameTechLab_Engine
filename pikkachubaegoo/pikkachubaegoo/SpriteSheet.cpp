#include "SpriteSheet.h"
#include "SpriteSheet.h"
#include "SpriteSheet.h"
#include <fstream>
#include <iostream>
#include <Windows.h>
#include "SpriteSheet.h"
#include "json.hpp"
#include "SpriteSheet.h"

USpriteSheet::USpriteSheet()
{
}

USpriteSheet::~USpriteSheet()
{
}

bool USpriteSheet::Load(std::string jsonPath)
{
    using json = nlohmann::json;
    spriteFramePath = jsonPath;

    std::ifstream fp(spriteFramePath);
    if (!fp.is_open())
    {
        std::cerr << "file open fail";
        return false;
    }

    json frameJson;
    fp >> frameJson;

    // 메타데이터 추출
    if (frameJson.contains("meta"))
    {
        // 디렉토리 경로 추출
        std::string directoryPath = "";
        size_t slashPosition = spriteFramePath.find_last_of("/\\");        
        if (slashPosition != std::string::npos)
        {
            directoryPath = spriteFramePath.substr(0, slashPosition + 1);
        }

        const auto& meta = frameJson["meta"];

        std::string imageFileName = meta["image"];
        metaData.imagePath = directoryPath + imageFileName;    
        metaData.size = {
            meta["size"]["w"],
            meta["size"]["h"]
        };
        metaData.scale = std::stof(meta["scale"].get<std::string>());

        spriteImagePath = ConvertStringToWstring(metaData.imagePath);
    }
    
    // spriteframe 추출
    for (auto& item : frameJson["frames"].items())
    {
        const std::string& key = item.key();
        const auto& value = item.value();

        FSpriteFrame newFrame;
        newFrame.name = key;
        
        if (value.contains("frame"))
        {
            const auto& data = value["frame"];
            newFrame.position = { data["x"], data["y"] };
            newFrame.size = { data["w"], data["h"] };
        }
        
        if (value.contains("rotated"))
        {
            newFrame.isRotate = value["rotated"].get<bool>();
        }

        if (value.contains("trimmed"))
        {
            newFrame.isTrim = value["trimmed"].get<bool>();
        }

        if (value.contains("spriteSourceSize"))
        {            
            newFrame.spriteSourceSize = { 
                value["spriteSourceSize"]["x"],
                value["spriteSourceSize"]["y"],
                value["spriteSourceSize"]["w"],
                value["spriteSourceSize"]["h"]
            };
        }

        if (value.contains("sourceSize"))
        {
            newFrame.sourceSize = { value["sourceSize"]["w"], value["sourceSize"]["h"] };

        }

        frameDatas[key] = newFrame;
    }

    return true;
}

const FSpriteFrame* USpriteSheet::GetFrame(const std::string& spriteName) const
{
    std::map<std::string, FSpriteFrame>::const_iterator it = frameDatas.find(spriteName);

    if (it == frameDatas.end())
    {
        std::cerr << "invalid spritename";
        return nullptr;
    }

    return &it->second;
}

const FVector4 USpriteSheet::GetSourceRect(const std::string& spriteName) const
{
    FVector4 rect;

    const FSpriteFrame* frame = this->GetFrame(spriteName);
    if (frame)
    {
        float x, y, w, h;
        x = frame->position.x;
        y = frame->position.y;
        w = frame->size.x;
        h = frame->size.y;
        rect = { x, y, w, h };
    }

    return rect;
}

std::wstring USpriteSheet::ConvertStringToWstring(std::string& source)
{
    if (source.empty())
    {
        return L"";
    }
    int size = MultiByteToWideChar(CP_UTF8, 0, &source[0], (int)source.size(), NULL, 0);

    std::wstring wstr(size, 0);

    MultiByteToWideChar(CP_UTF8, 0, &source[0], (int)source.size(), &wstr[0], size);

    return wstr;
}
