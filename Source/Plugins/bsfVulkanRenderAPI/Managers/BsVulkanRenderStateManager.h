//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "Managers/BsRenderStateManager.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/**	Handles creation of Vulkan pipeline states. */
		class VulkanRenderStateManager : public RenderStateManager
		{
		protected:
			SPtr<SamplerState> CreateSamplerStateInternalInternal(const SamplerStateCreateInformation& desc) const override;
		};

		/** @} */
	} // namespace ct
} // namespace bs
