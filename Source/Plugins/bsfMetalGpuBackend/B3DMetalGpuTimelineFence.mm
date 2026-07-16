//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuTimelineFence.h"
#include "B3DMetalGpuDevice.h"

namespace b3d
{
	namespace render
	{
		struct MetalGpuTimelineFence::Impl
		{
			id<MTLSharedEvent> Event = nil;

			/**
			 * Listener used by WaitInternal's native blocking wait and reused across waits to avoid
			 * repeated listener allocation. Initialized with a dedicated dispatch queue:
			 * the default @c [MTLSharedEventListener init] delivers notification blocks on the main
			 * dispatch queue, which deadlocks any main-thread Wait() (main would block on a semaphore
			 * only signaled by a listener block that can never run while main is blocked). Same
			 * pattern as MetalGpuQueue::Impl::EventListener.
			 */
			MTLSharedEventListener* Listener = nil;

			/**
			 * Dispatch queue delivering the listener's notification blocks. Serial is sufficient: a
			 * fence services one blocking waiter at a time and the callback is a trivial semaphore
			 * signal.
			 */
			dispatch_queue_t ListenerDispatchQueue = nullptr;
		};

		MetalGpuTimelineFence::MetalGpuTimelineFence(MetalGpuDevice& device)
			: mImpl(B3DMakeUnique<Impl>())
		{
			id<MTLDevice> mtlDevice = device.GetMetalDevice();
			if (mtlDevice == nil)
				return;

			mImpl->Event = [mtlDevice newSharedEvent];
			if (mImpl->Event == nil)
				return;

			mImpl->ListenerDispatchQueue = dispatch_queue_create("b3d.metal.fencelistener", DISPATCH_QUEUE_SERIAL);
			if (mImpl->ListenerDispatchQueue != nullptr)
				mImpl->Listener = [[MTLSharedEventListener alloc] initWithDispatchQueue:mImpl->ListenerDispatchQueue];
		}

		MetalGpuTimelineFence::~MetalGpuTimelineFence()
		{
			if (mImpl)
			{
				mImpl->Listener = nil;
				mImpl->Event = nil;

				// Under MRC, dispatch_queue_t is not ARC-managed and leaks if not released explicitly.
				// Under ARC it is toll-free bridged as an Obj-C object and released automatically;
				// calling @c dispatch_release under ARC is prohibited. Feature-guard to match the
				// pattern used in MetalGpuQueue's destructor.
#if !__has_feature(objc_arc)
				if (mImpl->ListenerDispatchQueue != nullptr)
					dispatch_release(mImpl->ListenerDispatchQueue);
#endif
				mImpl->ListenerDispatchQueue = nullptr;
			}
		}

		u64 MetalGpuTimelineFence::GetCompletedValue() const
		{
			if (mImpl->Event == nil)
				return 0;

			return (u64)[mImpl->Event signaledValue];
		}

		void MetalGpuTimelineFence::WaitInternal(u64 value)
		{
			// A fence with no event can never signal; bail instead of parking a worker thread forever
			// (the base poll would spin on GetCompletedValue() == 0 indefinitely).
			if (mImpl->Event == nil)
				return;

			// Listener construction failed but the event exists: the base poll-with-sleep still
			// observes signaledValue correctly, so fall back to it.
			if (mImpl->Listener == nil)
			{
				GpuTimelineFence::WaitInternal(value);
				return;
			}

			if ((u64)[mImpl->Event signaledValue] >= value)
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
			[mImpl->Event notifyListener:mImpl->Listener atValue:value block:^(id<MTLSharedEvent>, uint64_t)
			{
				dispatch_semaphore_signal(sem);
			}];
			dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);

			// See destructor note: MRC requires an explicit release; ARC forbids it.
#if !__has_feature(objc_arc)
			dispatch_release(sem);
#endif
		}

		id<MTLSharedEvent> MetalGpuTimelineFence::GetSharedEvent() const
		{
			return mImpl->Event;
		}
	} // namespace render
} // namespace b3d
