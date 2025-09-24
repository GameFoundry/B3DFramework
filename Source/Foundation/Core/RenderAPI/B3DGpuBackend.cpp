//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuBackend.h"
#include "B3DGpuFrameCapture.h"

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
