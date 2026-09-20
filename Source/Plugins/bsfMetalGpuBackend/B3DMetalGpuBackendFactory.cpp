//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuBackendFactory.h"
#include "B3DMetalGpuBackend.h"
#include "CoreObject/B3DRenderThread.h"

namespace b3d
{
	namespace render
	{
		constexpr const char* MetalGpuBackendFactory::SystemName;

		void MetalGpuBackendFactory::Create()
		{
			auto fnStartUp = []() { // TODO - Not quite ready to be started from the main thread as command buffer pools gets bound to the calling thread
				GpuBackend::StartUp<MetalGpuBackend>();
			};

			RenderThread::Instance().PostCommand(fnStartUp, "MetalGpuBackend::StartUp", true);
		}
	} // namespace render
} // namespace b3d
