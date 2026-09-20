//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Managers/B3DGpuBackendManager.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**	Handles creation of the MetalGpuBackend. */
		class MetalGpuBackendFactory : public GpuBackendFactory
		{
		public:
			static constexpr const char* SystemName = "bsfMetalGpuBackend";

			void Create() override;
			const char* Name() const override { return SystemName; }
		};

		/** @} */
	} // namespace render
} // namespace b3d
