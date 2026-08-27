//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullRenderer.h"
#include "CoreObject/B3DRenderThread.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "Renderer/B3DRendererManager.h"
#include "Renderer/B3DRendererUtility.h"

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
	GetRenderThread().PostCommand([this]()
	{
		ActivateOnRenderThread();

		RendererUtility::StartUp();
	}, "NullRenderer::ActivateOnRenderThread");
}

void render::NullRenderer::Destroy()
{
	Renderer::Destroy();

	GetRenderThread().PostCommand([this]()
	{
		RendererUtility::ShutDown();
		DestroyOnRenderThread();
	}, "NullRenderer::DestroyOnRenderThread", true);
}

void render::NullRenderer::RenderAll(PerFrameData perFrameData)
{
	// No scenes to render, but the frame boundary must still be pumped and screen captures resolved
	// for windows rendered outside of the renderer (e.g. low-level rendering examples)
	GetRenderThread().PostCommand([this]()
	{
		BeginFrame();
		ResolveOutstandingScreenCaptures();
		EndFrame();

		GpuUniformBufferManager::Instance().AdvanceFrame();
		mCommandBufferPoolRing->AdvanceFrame();
	}, "NullRenderer::RenderAll");
}

TShared<render::RendererScene> render::NullRenderer::CreateScene()
{
	return B3DMakeShared<render::NullRendererScene>();
}

TShared<render::NullRenderer> render::GetNullRenderer()
{
	return std::static_pointer_cast<render::NullRenderer>(RendererManager::Instance().GetActive());
}
