//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DD3D12RenderWindowManager.h"
#include "B3DD3D12HeadlessRenderWindowSurface.h"
#include "B3DD3D12RenderWindowSurface.h"

using namespace b3d;

TShared<render::IRenderWindowSurface> D3D12RenderWindowManager::CreateRenderWindowSurface(const render::RenderWindowSurfaceCreateInformation& createInformation)
{
	if (createInformation.Headless)
		return B3DMakeShared<render::D3D12HeadlessRenderWindowSurface>(createInformation);

	return B3DMakeShared<render::D3D12RenderWindowSurface>(createInformation);
}
