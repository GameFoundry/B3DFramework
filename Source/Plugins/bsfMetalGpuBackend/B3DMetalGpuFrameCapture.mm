//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuFrameCapture.h"
#include "B3DMetalGpuDevice.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	struct MetalGpuFrameCapture::Impl
	{
		bool Capturing = false;
		NSURL* OutputUrl = nil;
	};

	MetalGpuFrameCapture::MetalGpuFrameCapture(render::MetalGpuDevice& device)
		: mDevice(device)
		, mImpl(B3DMakeUnique<Impl>())
	{ }

	MetalGpuFrameCapture::~MetalGpuFrameCapture()
	{
		Stop();
#if !__has_feature(objc_arc)
		[mImpl->OutputUrl release];
#endif
		mImpl->OutputUrl = nil;
	}

	void MetalGpuFrameCapture::Start()
	{
		if (mImpl->Capturing)
			return;

		@autoreleasepool
		{
#if !__has_feature(objc_arc)
			[mImpl->OutputUrl release];
#endif
			mImpl->OutputUrl = nil;

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
#if !__has_feature(objc_arc)
					[mImpl->OutputUrl release];
					mImpl->OutputUrl = [descriptor.outputURL retain];
#else
					mImpl->OutputUrl = descriptor.outputURL;
#endif
				}
			}

#if !__has_feature(objc_arc)
			[descriptor release];
#endif
			mImpl->Capturing = started;
			if (!started)
			{
				B3D_LOG(Error, LogRenderBackend, "Failed to start Metal GPU capture: {0}",
					error ? String([[error localizedDescription] UTF8String]) : String("No supported capture destination."));
			}
		}
	}

	void MetalGpuFrameCapture::Stop()
	{
		if (!mImpl->Capturing)
			return;

		@autoreleasepool
		{
			[[MTLCaptureManager sharedCaptureManager] stopCapture];
			mImpl->Capturing = false;
			if (mImpl->OutputUrl != nil)
				B3D_LOG(Info, LogRenderBackend, "Metal GPU capture saved to {0}.", String([[mImpl->OutputUrl path] UTF8String]));
#if !__has_feature(objc_arc)
			[mImpl->OutputUrl release];
#endif
			mImpl->OutputUrl = nil;
		}
	}
} // namespace b3d
