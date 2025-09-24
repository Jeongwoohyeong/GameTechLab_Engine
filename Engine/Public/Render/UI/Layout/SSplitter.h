#pragma once
#include "Render/UI/Layout/SWindow.h"

// Splitter base class. Owns two child windows and divides Rect using SplitRatio.
class SSplitter : public SWindow
{
public:
	explicit SSplitter(float InSplitRatio = 0.5f)
		: SplitRatio(InSplitRatio) {
	}
	virtual ~SSplitter();

	void SetChildren(SWindow* LT, SWindow* RB);
	SWindow* GetLT() const { return SideLT; }
	SWindow* GetRB() const { return SideRB; }

	void SetSplitRatio(float r) { SplitRatio = std::clamp(r, 0.05f, 0.95f); }
	float GetSplitRatio() const { return SplitRatio; }

	void SetRect(const FRect& InRect) override { SWindow::SetRect(InRect); LayoutChildren(); }

	void Tick(float dt) override;
	void Draw() override;

	// Each derived splitter computes children rects and recurses
	void LayoutChildren() override {};

	bool IsSplitter() const override { return true; }

	// Drag support
	virtual bool HitTestHandle(FPoint p) const = 0;
	virtual void DragTo(FPoint p) = 0;
	bool IsDragging() const { return bDragging; }
	void SetDragging(bool b) { bDragging = b; }

protected:
	SWindow* SideLT{ nullptr }; // Left or Top
	SWindow* SideRB{ nullptr }; // Right or Bottom
	float    SplitRatio{ 0.5f };
	bool     bDragging{ false };
	static constexpr float HANDLE_THICKNESS = 4.0f; // px
};
