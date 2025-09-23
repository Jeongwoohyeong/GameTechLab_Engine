#pragma once

class FViewport;

class FViewportClient
{
public:
	virtual ~FViewportClient() = default;

	// Called every frame to draw the viewport
	virtual void Draw(FViewport& Viewport) = 0;
	// Called when viewport rect changes
	virtual void OnResize(FViewport& /*Viewport*/, int /*Width*/, int /*Height*/) {}
};
