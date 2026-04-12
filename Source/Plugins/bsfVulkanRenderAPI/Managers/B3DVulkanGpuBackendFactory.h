//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Managers/B3DGpuBackendManager.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/**	Handles creation of the VulkanGpuBackend. */
		class VulkanGpuBackendFactory : public GpuBackendFactory
		{
		public:
			static constexpr const char* SystemName = "bsfVulkanRenderAPI";

			void Create() override;
			const char* Name() const override { return SystemName; }
		};

		/** @} */
	} // namespace render
} // namespace b3d
