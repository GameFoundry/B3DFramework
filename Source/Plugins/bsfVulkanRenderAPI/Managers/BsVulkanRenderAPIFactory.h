//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Managers/BsRenderAPIFactory.h"
#include "Managers/BsRenderAPIManager.h"
#include "BsVulkanRenderAPI.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/**	Handles creation of the Vulkan render system. */
		class VulkanRenderAPIFactory : public RenderAPIFactory
		{
		public:
			static constexpr const char* SystemName = "bsfVulkanRenderAPI";

			/** @copydoc RenderAPIFactory::create */
			void Create();

			/** @copydoc RenderAPIFactory::name */
			const char* Name() const { return SystemName; }

		private:
			/**	Registers the factory with the render system manager when constructed. */
			class InitOnStart
			{
			public:
				InitOnStart()
				{
					static SPtr<RenderAPIFactory> newFactory;
					if(newFactory == nullptr)
					{
						newFactory = bs_shared_ptr_new<VulkanRenderAPIFactory>();
						RenderAPIManager::Instance().RegisterFactory(newFactory);
					}
				}
			};

			static InitOnStart initOnStart; // Makes sure factory is registered on program start
		};

		/** @} */
	} // namespace ct
} // namespace bs
