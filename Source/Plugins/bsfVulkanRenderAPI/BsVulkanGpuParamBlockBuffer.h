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
		class VulkanGpuParamBlockBuffer final : public GpuParamBlockBuffer
		{
		public:
			VulkanGpuParamBlockBuffer(u32 size, GpuBufferFlags flags, GpuDeviceFlags deviceMask);
			VulkanGpuParamBlockBuffer(const SPtr<GpuBuffer>& backingMemory, u32 offset, u32 size);

			/**
			 * Gets the resource wrapping the buffer object, on the specified device. If GPU param block buffer's device mask
			 * doesn't include the provided device, null is returned.
			 */
			VulkanBuffer* GetResource(u32 deviceIdx) const;
		};

		/** @} */
	} // namespace ct
} // namespace bs
