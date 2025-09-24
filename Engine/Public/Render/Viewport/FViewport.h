#pragma once
#include "Render/UI/Layout/SWindow.h"
#include "Render/Viewport/FViewportClient.h"

class FViewport
{
public:
	explicit FViewport(FViewportClient* InClient = nullptr)
		: Client(InClient) {
	}

	void SetClient(FViewportClient* InClient) { Client = InClient; }
	FViewportClient* GetClient() const { return Client; }

	void SetRect(const FRect& InRect)
	{
		Rect = InRect;
		if (Client) Client->OnResize(*this, static_cast<int>(Rect.W), static_cast<int>(Rect.H));
	}

	const FRect& GetRect() const { return Rect; }

	void Present()
	{
		if (Client) Client->Draw(*this);
	}

private:
	FRect Rect{};
	FViewportClient* Client{ nullptr }; // Not owned
};
