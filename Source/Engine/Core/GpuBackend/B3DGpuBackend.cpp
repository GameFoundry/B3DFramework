//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuBackend.h"
#include "B3DGpuCommandCapture.h"

namespace b3d
{
	TConfigVariable<bool> gGpuPreferIntegrated("gpu.PreferIntegrated",
		"Prefer using integrated GPU over discrete GPU when both are available.", false, ConfigVariableFlag::ReadOnly);
	TConfigVariable<i32> gGpuPreferredDeviceIndex("gpu.PreferredDeviceIndex",
		"Specifies the index of the GPU to use. If < 0 is provided, best GPU is selected automatically.", -1,
		ConfigVariableFlag::ReadOnly);
	TConfigVariable<u32> gGpuBindlessResourceCapacity("gpu.BindlessResourceCapacity",
		"Number of slots in the bindless resource heap. Clamped to the device limit.", 256 * 1024, ConfigVariableFlag::ReadOnly);
	TConfigVariable<u32> gGpuBindlessSamplerCapacity("gpu.BindlessSamplerCapacity",
		"Number of slots in the bindless sampler heap. Clamped to the device limit.", 1024, ConfigVariableFlag::ReadOnly);
	TConfigVariable<float> gGpuBindlessSamplerEvictionThreshold("gpu.BindlessSamplerEvictionThreshold",
		"Fraction of the bindless sampler heap that, once allocated, makes new sampler slots first evict cached samplers nothing else references.", 0.9f);

#if B3D_BUILD_TYPE_DEVELOPMENT
	TConfigVariable<bool> gGpuDumpPipelineTimings("gpu.DumpPipelineTimings",
		"Log the driver pipeline-compile time (ms) at compute pipeline creation. Clear the driver shader caches first for "
		"cold (true compile) numbers; a warm cache returns in <1 ms.",
		false, ConfigVariableFlag::ReadOnly);
#endif
} // namespace b3d

using namespace b3d;

void GpuBackend::StartGPUCommandCapture()
{
	if (mGPUCommandCapture)
		mGPUCommandCapture->Start();
}

void GpuBackend::StopGPUCommandCapture()
{
	if (mGPUCommandCapture)
		mGPUCommandCapture->Stop();
}
