//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanGpuBackendFactory.h"
#include "BsVulkanGpuBackend.h"
#include "CoreThread/BsCoreThread.h"

using namespace bs;
using namespace bs::ct;

constexpr const char* VulkanGpuBackendFactory::SystemName;

void VulkanGpuBackendFactory::Create()
{
	auto fnStartUp = []() { // TODO - Not quite ready to be started from the main thread as command buffer pools gets bound to the calling thread
		GpuBackend::StartUp<VulkanGpuBackend>();
	};

	CoreThread::Instance().QueueCommand(fnStartUp, CTQF_InternalQueue | CTQF_BlockUntilComplete);
}

VulkanGpuBackendFactory::InitOnStart VulkanGpuBackendFactory::initOnStart;
