//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanGpuBackend.h"
#include "RenderAPI/BsRenderAPI.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Implementation of a render system using Vulkan. Provides abstracted access to various low level Vulkan methods. */
		class VulkanRenderAPI : public RenderAPI
		{
		public:
			VulkanRenderAPI() = default;
			~VulkanRenderAPI() override = default;

			const StringID& GetName() const override;
			void BeginFrame() override;
			void EndFrame() override;
			SPtr<GpuDevice> GetPrimaryGpuDevice() const override { return mPrimaryGpuDevice; }
		protected:
			friend class VulkanRenderAPIFactory;

			void Initialize() override;
			void DestroyCore() override;

		private:
			SPtr<GpuDevice> mPrimaryGpuDevice;
		};

		/**	Provides easy access to the VulkanRenderAPI. */
		VulkanRenderAPI& GetVulkanRenderAPI();

		/** @} */
	} // namespace ct
} // namespace bs
