//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuTimelineFence.h"
#include "B3DMetalGpuDevice.h"

namespace b3d
{
	namespace render
	{
		MetalGpuTimelineFence::MetalGpuTimelineFence(MetalGpuDevice& device)
			: mDevice(device)
		{
			id<MTLDevice> mtlDevice = device.GetMetalDevice();
			if (mtlDevice == nil)
				return;

			mEvent = [mtlDevice newSharedEvent];
		}

		u64 MetalGpuTimelineFence::GetCompletedValue() const
		{
			if (mEvent == nil)
				return 0;

			return (u64)[mEvent signaledValue];
		}

		void MetalGpuTimelineFence::WaitInternal(u64 value)
		{
			if (mEvent == nil)
				return;

			MTLSharedEventListener* listener = mDevice.GetSharedEventListener();
			if (listener == nil)
			{
				GpuTimelineFence::WaitInternal(value);
				return;
			}

			if ((u64)[mEvent signaledValue] >= value)
				return;

			// MTLSharedEvent has no native blocking CPU wait; block on a semaphore signaled by the
			// listener. Per Apple's contract, notifyListener:atValue: invokes the block immediately
			// if the event has already reached the value, so the pre-check above is only a fast path
			// and the wait below cannot miss the signal.
			dispatch_semaphore_t sem = dispatch_semaphore_create(0);
			if (sem == nullptr)
			{
				GpuTimelineFence::WaitInternal(value);
				return;
			}

			[mEvent notifyListener:listener atValue:value block:^(id<MTLSharedEvent>, uint64_t)
			{
				dispatch_semaphore_signal(sem);
			}];
			dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
		}

	} // namespace render
} // namespace b3d
