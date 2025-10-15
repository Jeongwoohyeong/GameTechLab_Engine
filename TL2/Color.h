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
    /**
     * @brief 두 FLinearColor 사이를 선형 보간
     * @param A 시작 색상
     * @param B 끝 색상
     * @param Alpha 보간 계수 (0.0 = A, 1.0 = B)
     * @return 보간된 색상
     */
    static FLinearColor Lerp(const FLinearColor& A, const FLinearColor& B, float Alpha)
    {
        // Alpha 값을 0.0 ~ 1.0 사이로 제한하여 안정성을 높입니다.
        const float ClampedAlpha = std::max(0.0f, std::min(1.0f, Alpha));
        return A * (1.0f - ClampedAlpha) + B * ClampedAlpha;
    }
};


inline const FLinearColor FLinearColor::White{ 1.0f, 1.0f, 1.0f, 1.0f };
inline const FLinearColor FLinearColor::Black{ 0.0f, 0.0f, 0.0f, 1.0f };
inline const FLinearColor FLinearColor::Red{ 1.0f, 0.0f, 0.0f, 1.0f };
inline const FLinearColor FLinearColor::Green{ 0.0f, 1.0f, 0.0f, 1.0f };
inline const FLinearColor FLinearColor::Blue{ 0.0f, 0.0f, 1.0f, 1.0f };
