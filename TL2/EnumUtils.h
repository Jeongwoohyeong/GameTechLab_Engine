#pragma once
#include <type_traits>

template<typename EnumClass>
constexpr auto ToUnderlying(EnumClass EnumVariable) noexcept
{
    return static_cast<std::underlying_type_t<EnumClass>>(EnumVariable);
}

template<typename EnumClass>
constexpr EnumClass& operator++(EnumClass& Enum) noexcept
{
    static_assert(std::is_enum_v<EnumClass>, "This class must be an enum.");

    Enum = static_cast<EnumClass>(ToUnderlying(Enum) + 1);
    
    return Enum;
}

template<typename EnumClass>
constexpr EnumClass operator++(EnumClass& Enum, int Dummy) noexcept
{
    static_assert(std::is_enum_v<EnumClass>, "This class must be an enum.");
    EnumClass OriginalEnum = Enum;
    ++Enum;

    return OriginalEnum;
}

template<typename EnumClass>
constexpr EnumClass operator%(EnumClass LHS, EnumClass RHS) noexcept
{
    static_assert(std::is_enum_v<EnumClass>, "This class must be an enum.");
    return static_cast<EnumClass>(ToUnderlying(LHS) % ToUnderlying(RHS));
}