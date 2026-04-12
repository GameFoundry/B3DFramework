//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DVulkanGpuBackendFactory.h"
#include "B3DVulkanGpuBackend.h"
#include "CoreObject/B3DRenderThread.h"

using namespace b3d;
using namespace b3d::render;

constexpr const char* VulkanGpuBackendFactory::SystemName;

void VulkanGpuBackendFactory::Create()
{
	auto fnStartUp = []() { // TODO - Not quite ready to be started from the main thread as command buffer pools gets bound to the calling thread
		GpuBackend::StartUp<VulkanGpuBackend>();
	};

	RenderThread::Instance().PostCommand(fnStartUp, "VulkanGpuBackend::StartUp", true);
}
