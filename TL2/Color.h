#pragma once

struct FLinearColor;

struct FLinearColor
{
    float R, G, B, A;

    FLinearColor(float InR = 0.0f, float InG = 0.0f, float InB = 0.0f, float InA = 1.0f)
        : R(InR), G(InG), B(InB), A(InA)
    {
    }

    static const FLinearColor White;
    static const FLinearColor Black;
    static const FLinearColor Red;
    static const FLinearColor Green;
    static const FLinearColor Blue;
};

inline const FLinearColor FLinearColor::White{ 1.0f, 1.0f, 1.0f, 1.0f };
inline const FLinearColor FLinearColor::Black{ 0.0f, 0.0f, 0.0f, 1.0f };
inline const FLinearColor FLinearColor::Red{ 1.0f, 0.0f, 0.0f, 1.0f };
inline const FLinearColor FLinearColor::Green{ 0.0f, 1.0f, 0.0f, 1.0f };
inline const FLinearColor FLinearColor::Blue{ 0.0f, 0.0f, 1.0f, 1.0f };
