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

    // ======== Operator =======
    FLinearColor operator+(const FLinearColor& Other) const
    {
        return FLinearColor(R + Other.R, G + Other.G, B + Other.B, A + Other.A);
    }
    FLinearColor operator-(const FLinearColor& Other) const
    {
        return FLinearColor(R - Other.R, G - Other.G, B - Other.B, A - Other.A);
    }
    FLinearColor operator*(const FLinearColor& Other) const
    {
        return FLinearColor(R * Other.R, G * Other.G, B * Other.B, A * Other.A);
    }
    FLinearColor operator*(float Scalar) const
    {
        return FLinearColor(R * Scalar, G * Scalar, B * Scalar, A * Scalar);
    }
    FLinearColor operator/(float Scalar) const
    {
        const float InvScalar = 1.0f / Scalar;
        return FLinearColor(R * InvScalar, G * InvScalar, B * InvScalar, A * InvScalar);
    }
    FLinearColor& operator+=(const FLinearColor& Other)
    {
        R += Other.R; G += Other.G; B += Other.B; A += Other.A;
        return *this;
    }
    FLinearColor& operator*=(float Scalar)
    {
        R *= Scalar; G *= Scalar; B *= Scalar; A *= Scalar;
        return *this;
    }
    bool operator==(const FLinearColor& Other) const
    {
        // Use a small tolerance (epsilon) for robust float comparison
        return std::abs(R - Other.R) < 1e-4f &&
            std::abs(G - Other.G) < 1e-4f &&
            std::abs(B - Other.B) < 1e-4f &&
            std::abs(A - Other.A) < 1e-4f;
    }
    bool operator!=(const FLinearColor& Other) const
    {
        return !(*this == Other);
    }
    // ========== Utility ==========

};


inline const FLinearColor FLinearColor::White{ 1.0f, 1.0f, 1.0f, 1.0f };
inline const FLinearColor FLinearColor::Black{ 0.0f, 0.0f, 0.0f, 1.0f };
inline const FLinearColor FLinearColor::Red{ 1.0f, 0.0f, 0.0f, 1.0f };
inline const FLinearColor FLinearColor::Green{ 0.0f, 1.0f, 0.0f, 1.0f };
inline const FLinearColor FLinearColor::Blue{ 0.0f, 0.0f, 1.0f, 1.0f };
