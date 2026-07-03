//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "Managers/B3DRenderWindowManager.h"

namespace b3d
{
	/** @addtogroup D3D12GpuBackend
	 *  @{
	 */

	/** @copydoc RenderWindowManager */
	class D3D12RenderWindowManager : public RenderWindowManager
	{
	public:
		D3D12RenderWindowManager() = default;

		TShared<render::IRenderWindowSurface> CreateRenderWindowSurface(const render::RenderWindowSurfaceCreateInformation& createInformation) override;
	};

	/** @} */
} // namespace b3d
