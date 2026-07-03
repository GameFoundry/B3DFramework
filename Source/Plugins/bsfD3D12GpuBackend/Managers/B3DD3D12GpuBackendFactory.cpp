//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuBackendFactory.h"
#include "B3DD3D12GpuBackend.h"
#include "CoreObject/B3DRenderThread.h"

using namespace b3d;

constexpr const char* D3D12GpuBackendFactory::SystemName;

void D3D12GpuBackendFactory::Create()
{
	auto fnStartUp = []() { // TODO - Not quite ready to be started from the main thread as command buffer pools gets bound to the calling thread
		GpuBackend::StartUp<D3D12GpuBackend>();
	};

	RenderThread::Instance().PostCommand(fnStartUp, "D3D12GpuBackend::StartUp", true);
}
