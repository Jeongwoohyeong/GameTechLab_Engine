#include "Animation.h"
#include <vector>

UAnimation::UAnimation()
    : clipName({
    {
        {
            "pikachu/pikachu_4",
            {
                "pikachu/pikachu_0_0.png"
            }
        },
        {
            "pikachu/pikachu_0",
            {
                "pikachu/pikachu_0_0.png",
                "pikachu/pikachu_0_1.png",
                "pikachu/pikachu_0_2.png",
                "pikachu/pikachu_0_3.png",
                "pikachu/pikachu_0_4.png"
            }
        },
        {
            "pikachu/pikachu_1",
            {
                "pikachu/pikachu_1_0.png",
                "pikachu/pikachu_1_1.png",
                "pikachu/pikachu_1_2.png",
                "pikachu/pikachu_1_3.png",
                "pikachu/pikachu_1_4.png"
            }
        },
        {
            "pikachu/pikachu_2",
            {
                "pikachu/pikachu_2_0.png",
                "pikachu/pikachu_2_1.png",
                "pikachu/pikachu_2_2.png",
                "pikachu/pikachu_2_3.png",
                "pikachu/pikachu_2_4.png"
            }
        },
        {
            "pikachu/pikachu_3",
            {
                "pikachu/pikachu_3_0.png",
                "pikachu/pikachu_3_1.png"
            }
        },
        {
            "pikachu/pikachu_5",
            {
                "pikachu/pikachu_5_0.png",
                "pikachu/pikachu_5_1.png",
                "pikachu/pikachu_5_2.png",
                "pikachu/pikachu_5_3.png",
                "pikachu/pikachu_5_4.png"
            }
        },
        {
            "pikachu/pikachu_6",
            {
                "pikachu/pikachu_3_1.png"
            }
        }
        }
        })
{ }

UAnimation::~UAnimation()
{
}

const std::vector<std::string>* UAnimation::GetClipList(const std::string& name) const
{
    auto it = clipName.find(name);
    if (it == clipName.end())
    {
        return nullptr;
    }

    return &it->second;
}
