//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuBackendFactory.h"
#include "B3DNullGpuBackend.h"
#include "CoreObject/B3DRenderThread.h"

using namespace b3d;

constexpr const char* NullGpuBackendFactory::SystemName;

void NullGpuBackendFactory::Create()
{
	auto fnStartUp = []() { // TODO - Not quite ready to be started from the main thread as command buffer pools gets bound to the calling thread
		GpuBackend::StartUp<NullGpuBackend>();
	};

	RenderThread::Instance().PostCommand(fnStartUp, "NullGpuBackend::StartUp", true);
}
