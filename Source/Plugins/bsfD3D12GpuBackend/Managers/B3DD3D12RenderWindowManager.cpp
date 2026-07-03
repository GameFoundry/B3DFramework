//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DD3D12RenderWindowManager.h"
#include "B3DD3D12RenderWindowSurface.h"

using namespace b3d;

TShared<render::IRenderWindowSurface> D3D12RenderWindowManager::CreateRenderWindowSurface(const render::RenderWindowSurfaceCreateInformation& createInformation)
{
	// TODO(d3d12-port): Support headless surfaces (offscreen swap-chain-less rendering), mirroring
	// VulkanHeadlessRenderWindowSurface.
	if (!B3D_ENSURE_LOG(!createInformation.Headless, "Headless render windows are not yet supported by the D3D12 backend."))
		return nullptr;

	return B3DMakeShared<render::D3D12RenderWindowSurface>(createInformation);
}
