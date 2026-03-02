//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullRenderer.h"
#include "CoreThread/B3DCoreThread.h"
#include "Renderer/B3DRendererManager.h"
#include "CoreThread/B3DCoreObjectManager.h"

using namespace b3d;
using namespace b3d::render;

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

const StringID& NullRenderer::GetName() const
{
	static StringID name = "NullRenderer";
	return name;
}

void NullRenderer::RenderAll(PerFrameData perFrameData)
{
	CoreObjectManager::Instance().SyncToCore();
}

SPtr<NullRenderer> GetNullRenderer()
{
	return std::static_pointer_cast<NullRenderer>(RendererManager::Instance().GetActive());
}
