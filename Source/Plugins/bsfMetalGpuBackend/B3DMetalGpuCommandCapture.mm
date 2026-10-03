//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuCommandCapture.h"
#include "B3DMetalGpuDevice.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	MetalGpuCommandCapture::MetalGpuCommandCapture(render::MetalGpuDevice& device)
		: mDevice(device)
	{ }

	MetalGpuCommandCapture::~MetalGpuCommandCapture()
	{
		Stop();
	}

	void MetalGpuCommandCapture::Start()
	{
		if (mCapturing)
			return;

		mOutputUrl = nil;

		MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
		MTLCaptureDescriptor* descriptor = [[MTLCaptureDescriptor alloc] init];
		descriptor.captureObject = mDevice.GetMetalDevice();

		NSError* error = nil;
		bool started = false;
		if ([captureManager supportsDestination:MTLCaptureDestinationDeveloperTools])
		{
			descriptor.destination = MTLCaptureDestinationDeveloperTools;
			started = [captureManager startCaptureWithDescriptor:descriptor error:&error];
		}

		if (!started && [captureManager supportsDestination:MTLCaptureDestinationGPUTraceDocument])
		{
			NSString* fileName = [NSString stringWithFormat:@"Banshee-%@.gputrace", [[NSUUID UUID] UUIDString]];
			NSString* outputPath = [NSTemporaryDirectory() stringByAppendingPathComponent:fileName];
			descriptor.destination = MTLCaptureDestinationGPUTraceDocument;
			descriptor.outputURL = [NSURL fileURLWithPath:outputPath];
			error = nil;
			started = [captureManager startCaptureWithDescriptor:descriptor error:&error];
			if (started)
			{
				mOutputUrl = descriptor.outputURL;
			}
		}

		mCapturing = started;
		if (!started)
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to start Metal GPU capture: {0}",
				error ? String([[error localizedDescription] UTF8String]) : String("No supported capture destination."));
		}
	}

	void MetalGpuCommandCapture::Stop()
	{
		if (!mCapturing)
			return;

		[[MTLCaptureManager sharedCaptureManager] stopCapture];
		mCapturing = false;
		if (mOutputUrl != nil)
			B3D_LOG(Info, LogRenderBackend, "Metal GPU capture saved to {0}.", String([[mOutputUrl path] UTF8String]));
		mOutputUrl = nil;
	}
} // namespace b3d
