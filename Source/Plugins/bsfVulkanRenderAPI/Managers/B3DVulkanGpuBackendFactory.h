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

		private:
			/**	Registers the factory with the render system manager when constructed. */
			class InitOnStart
			{
			public:
				InitOnStart()
				{
					static SPtr<GpuBackendFactory> newFactory;
					if(newFactory == nullptr)
					{
						newFactory = B3DMakeShared<VulkanGpuBackendFactory>();
						GpuBackendManager::Instance().RegisterFactory(newFactory);
					}
				}
			};

			static InitOnStart initOnStart; // Makes sure factory is registered on program start
		};

		/** @} */
	} // namespace render
} // namespace b3d
