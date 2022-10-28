//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsGpuParamBlockBuffer.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/**	Vulkan implementation of a parameter block buffer (uniform buffer in Vulkan lingo). */
		class VulkanGpuParamBlockBuffer : public GpuParamBlockBuffer
		{
		public:
			VulkanGpuParamBlockBuffer(u32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask);
			~VulkanGpuParamBlockBuffer();

			/**
			 * Gets the resource wrapping the buffer object, on the specified device. If GPU param block buffer's device mask
			 * doesn't include the provided device, null is returned.
			 */
			VulkanBuffer* GetResource(u32 deviceIdx) const;

		protected:
			void Initialize() override;

		private:
			GpuDeviceFlags mDeviceMask;
		};

		/** @} */
	} // namespace ct
} // namespace bs
