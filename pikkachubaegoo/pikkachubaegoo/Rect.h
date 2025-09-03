#pragma once
#include "Vector.h"

struct FRect {
    FRect() : _min(FVector2(0, 0)), _max(FVector2(0, 0)) {}
    FRect(const FVector2& inMin, const FVector2& inMax) : _min(inMin), _max(inMax) {}
    FRect(const FVector2& center, float width, float height) 
    {
        _min.x = center.x - width / 2.0f;
        _min.y = center.y - height / 2.0f;
        _max.x = center.x + width / 2.0f;
        _max.y = center.y + height / 2.0f;
    }

    FVector2 _min; // Bottom-Left
    FVector2 _max; // Top-Right

    float GetWidth() const { return _max.x - _min.x; }
    float GetHeight() const { return _max.y - _min.y; }

    void SetCenter(const FVector2& newCenter)
    {
        float halfWidth = GetWidth() / 2.0f;
        float halfHeight = GetHeight() / 2.0f;

        _min.x = newCenter.x - halfWidth;
        _min.y = newCenter.y - halfHeight;
        _max.x = newCenter.x + halfWidth;
        _max.y = newCenter.y + halfHeight;
    }

    // AABB Collision Check
    bool Intersects(const FRect& other) const 
    {
        return !(_min.x > other._max.x || _max.x < other._min.x ||
            _min.y > other._max.y || _max.y < other._min.y);
    }
};
