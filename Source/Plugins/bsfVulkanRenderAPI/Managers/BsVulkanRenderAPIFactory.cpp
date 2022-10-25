//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanRenderAPIFactory.h"
#include "RenderAPI/BsRenderAPI.h"

namespace bs
{
	namespace ct
	{
		constexpr const char* VulkanRenderAPIFactory::SystemName;

		void VulkanRenderAPIFactory::Create()
		{
			RenderAPI::StartUp<VulkanRenderAPI>();
		}

		VulkanRenderAPIFactory::InitOnStart VulkanRenderAPIFactory::initOnStart;
	} // namespace ct
} // namespace bs
