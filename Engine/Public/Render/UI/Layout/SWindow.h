#pragma once
#include <algorithm>

// Simple geometry types used by layout
struct FPoint { float X{ 0 }, Y{ 0 }; };
struct FRect {
	float X{ 0 }, Y{ 0 }, W{ 0 }, H{ 0 };
	bool Contains(FPoint p) const { return p.X >= X && p.Y >= Y && p.X <= X + W && p.Y <= Y + H; }
};

class SWindow
{
public:
	virtual ~SWindow() = default;

	const FRect& GetRect() const { return Rect; }
	virtual void SetRect(const FRect& InRect) { Rect = InRect; }

	bool IsHover(FPoint coord) const { return Rect.Contains(coord); }

	// Hookss
	virtual void Tick(float /*dt*/) {}
	virtual void Draw() {}
	// Splitters override to layout children
	virtual void LayoutChildren() {}

	// Type query without RTTI (project disables RTTI)
	virtual bool IsSplitter() const { return false; }

protected:
	FRect Rect{};
};
