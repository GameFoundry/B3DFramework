//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuBackend.h"
#include "B3DGpuFrameCapture.h"

namespace b3d
{
	TConfigVariable<bool> gGpuPreferIntegrated("gpu.PreferIntegrated",
		"Prefer using integrated GPU over discrete GPU when both are available.", false, ConfigVariableFlag::ReadOnly);
	TConfigVariable<i32> gGpuPreferredDeviceIndex("gpu.PreferredDeviceIndex",
		"Specifies the index of the GPU to use. If < 0 is provided, best GPU is selected automatically.", -1,
		ConfigVariableFlag::ReadOnly);
} // namespace b3d

using namespace b3d;

void GpuBackend::StartCapture()
{
	if (mFrameCapture)
		mFrameCapture->Start();
}

void GpuBackend::StopCapture()
{
	if (mFrameCapture)
		mFrameCapture->Stop();
}
