//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullRenderer.h"
#include "CoreObject/B3DRenderThread.h"
#include "Renderer/B3DRendererManager.h"

using namespace b3d;

constexpr const char* NullRendererFactory::SystemName;

SPtr<render::Renderer> NullRendererFactory::Create()
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

void render::NullRenderer::Initialize(const SPtr<GpuDevice>& gpuDevice)
{
	Renderer::Initialize(gpuDevice);

	GetRenderThread().PostCommand([this]() { InitializeOnRenderThread(); }, "NullRenderer::InitializeOnRenderThread");
}

void render::NullRenderer::Destroy()
{
	Renderer::Destroy();

	GetRenderThread().PostCommand([this]() { DestroyOnRenderThread(); }, "NullRenderer::DestroyOnRenderThread", true);
}

void render::NullRenderer::RenderAll(PerFrameData perFrameData)
{
}

SPtr<render::RendererScene> render::NullRenderer::CreateScene()
{
	return B3DMakeShared<render::NullRendererScene>();
}

SPtr<render::NullRenderer> render::GetNullRenderer()
{
	return std::static_pointer_cast<render::NullRenderer>(RendererManager::Instance().GetActive());
}
