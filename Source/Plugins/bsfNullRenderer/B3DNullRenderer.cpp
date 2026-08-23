//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullRenderer.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DRenderWindow.h"
#include "Image/B3DColor.h"
#include "Image/B3DPixelData.h"
#include "Renderer/B3DRendererManager.h"

using namespace b3d;

constexpr const char* NullRendererFactory::SystemName;

TShared<render::Renderer> NullRendererFactory::Create()
{
	return B3DMakeShared<render::NullRenderer>();
}

const String& NullRendererFactory::Name() const
{
	static String StrSystemName = SystemName;
	return StrSystemName;
}

const StringID& render::NullRenderer::GetName() const
{
	static StringID name = "NullRenderer";
	return name;
}

void render::NullRenderer::Activate()
{
	GetRenderThread().PostCommand([this]() { ActivateOnRenderThread(); }, "NullRenderer::ActivateOnRenderThread");
}

void render::NullRenderer::Destroy()
{
	Renderer::Destroy();

	GetRenderThread().PostCommand([this]() { DestroyOnRenderThread(); }, "NullRenderer::DestroyOnRenderThread", true);
}

void render::NullRenderer::RenderAll(PerFrameData perFrameData)
{
}

void render::NullRenderer::RequestScreenCapture(const TShared<RenderWindow>& window, TAsyncOp<TShared<PixelData>> asyncOp)
{
	if(window == nullptr)
	{
		asyncOp.CompleteOperation(nullptr);
		return;
	}

	const RenderTargetProperties& properties = window->GetProperties();
	TShared<PixelData> pixelData = PixelData::Create(properties.Width, properties.Height, 1, PF_RGBA8);
	pixelData->SetColors(Color::kBlack);
	asyncOp.CompleteOperation(pixelData);
}

TShared<render::RendererScene> render::NullRenderer::CreateScene()
{
	return B3DMakeShared<render::NullRendererScene>();
}

TShared<render::NullRenderer> render::GetNullRenderer()
{
	return std::static_pointer_cast<render::NullRenderer>(RendererManager::Instance().GetActive());
}
