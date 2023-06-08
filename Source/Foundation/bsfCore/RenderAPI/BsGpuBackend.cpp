//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGpuBackend.h"
#include "BsGpuFrameCapture.h"

using namespace bs;

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
