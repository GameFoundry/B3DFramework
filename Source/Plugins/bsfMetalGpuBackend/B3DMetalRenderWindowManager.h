//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "Managers/B3DRenderWindowManager.h"

namespace b3d
{
	/** @addtogroup MetalGpuBackend
	 *  @{
	 */

	/** Metal implementation of RenderWindowManager. */
	class MetalRenderWindowManager : public RenderWindowManager
	{
	public:
		MetalRenderWindowManager() = default;
		~MetalRenderWindowManager() = default;

		TShared<render::IRenderWindowSurface> CreateRenderWindowSurface(const render::RenderWindowSurfaceCreateInformation& createInformation) override;
	};

	/** @} */
} // namespace b3d
