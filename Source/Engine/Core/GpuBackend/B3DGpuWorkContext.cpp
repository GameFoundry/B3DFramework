//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuWorkContext.h"
#include "B3DGpuDevice.h"

using namespace b3d;

GpuWorkContext::GpuWorkContext(GpuDevice& device)
	: mDevice(device), mTracker(nullptr), mOwnedTracker(B3DMakeUnique<GpuFenceCompletionTracker>(device.CreateTimelineFence()))
{
	mTracker = mOwnedTracker.get();
}

GpuWorkContext::GpuWorkContext(GpuDevice& device, IGpuCompletionTracker& primaryTracker)
	: mDevice(device), mTracker(&primaryTracker)
{
}

GpuWorkContext::~GpuWorkContext() = default;
