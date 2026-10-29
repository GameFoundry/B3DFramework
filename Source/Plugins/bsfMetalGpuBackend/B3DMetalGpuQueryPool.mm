//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuQueryPool.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuQueue.h"
#include "B3DMetalHeapAllocator.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	namespace render
	{
		MetalGpuQueryPool::MetalGpuQueryPool(MetalGpuDevice& gpuDevice, const GpuQueryPoolCreateInformation& createInformation)
			: GpuQueryPool(createInformation), mGpuDevice(gpuDevice)
		{
			id<MTLDevice> device = gpuDevice.GetMetalDevice();
			mResolvedResults.resize(mPoolSize);

			if (createInformation.Type == GpuQueryType::Occlusion)
			{
				if (createInformation.PoolSize > 0)
					mFirstVisibilityResultSlot = gpuDevice.AllocateVisibilityResultSlots(createInformation.PoolSize);

				mSupported = mFirstVisibilityResultSlot != RangeAllocator::kInvalidIndex;
				if (!mSupported)
				{
					B3D_LOG(Error, LogRenderBackend, "Metal occlusion query pool of size {0} does not fit in the free slots of the visibility result buffer, which has {1} for all pools.",
						createInformation.PoolSize, MetalGpuDevice::kVisibilityResultSlotCount);
				}
			}
			else if (createInformation.Type == GpuQueryType::Timestamp)
			{
				if (createInformation.PoolSize == 0)
					B3D_LOG(Error, LogRenderBackend, "Metal timestamp query pools cannot be empty.");
				else if (device != nil && gpuDevice.GetCapabilities().HasCapability(RSC_TIMER_QUERIES))
				{
					mFirstCounterHeapSlot = gpuDevice.AllocateCounterHeapSlots(createInformation.PoolSize, mCounterHeap);
					mSupported = mFirstCounterHeapSlot != RangeAllocator::kInvalidIndex;
				}
				else
					B3D_LOG(Warning, LogRenderBackend, "Timestamp query pool created on a device without RSC_TIMER_QUERIES; results will remain zero.");
			}
			else
				B3D_LOG(Warning, LogRenderBackend, "Pipeline-statistics query pools are unsupported by the Metal backend.");
		}

		MetalGpuQueryPool::~MetalGpuQueryPool()
		{
			// The pool is only destroyed once no command buffer that uses it is executing
			if (mFirstVisibilityResultSlot != RangeAllocator::kInvalidIndex)
				mGpuDevice.FreeVisibilityResultSlots(mFirstVisibilityResultSlot, mPoolSize);

			if (mFirstCounterHeapSlot != RangeAllocator::kInvalidIndex)
				mGpuDevice.FreeCounterHeapSlots(mCounterHeap, mFirstCounterHeapSlot, mPoolSize);
		}

		NSRange MetalGpuQueryPool::GetVisibilityResultRange() const
		{
			if (mFirstVisibilityResultSlot == RangeAllocator::kInvalidIndex)
				return NSMakeRange(0, 0);

			return NSMakeRange((NSUInteger)mFirstVisibilityResultSlot * sizeof(u64), (NSUInteger)mPoolSize * sizeof(u64));
		}

		GpuQueryId MetalGpuQueryPool::AllocateQuery()
		{
			Lock lock(mStateMutex);
			if (!mSupported || mNextQueryId >= mPoolSize)
				return GpuQueryId();

			mResultsCached = false;
			return GpuQueryId(mNextQueryId++);
		}

		void MetalGpuQueryPool::MarkRecorded()
		{
			Lock lock(mStateMutex);
			mRecordedCommandBuffers++;
		}

		void MetalGpuQueryPool::MarkQueuedForSubmission()
		{
			Lock lock(mStateMutex);
			if (!B3D_ENSURE(mRecordedCommandBuffers > 0))
				return;

			mRecordedCommandBuffers--;
			mPendingSubmissions++;
		}

		void MetalGpuQueryPool::MarkSubmissionFailed()
		{
			{
				Lock lock(mStateMutex);
				if (!B3D_ENSURE(mPendingSubmissions > 0))
					return;

				mPendingSubmissions--;
				mSubmissionFailed = true;
			}

			mSubmissionCondition.notify_all();
		}

		void MetalGpuQueryPool::MarkRecordingAbandoned()
		{
			Lock lock(mStateMutex);
			if (!B3D_ENSURE(mRecordedCommandBuffers > 0))
				return;

			mRecordedCommandBuffers--;
			mSubmissionFailed = true;
		}

		void MetalGpuQueryPool::MarkSubmitted(MetalGpuQueue& queue, u64 eventValue)
		{
			Lock lock(mStateMutex);
			if (!B3D_ENSURE(mPendingSubmissions > 0))
				return;

			mPendingSubmissions--;
			for (auto& entry : mInFlightSubmissions)
			{
				if (entry.first != &queue)
					continue;

				entry.second = std::max(entry.second, eventValue);
				lock.unlock();
				mSubmissionCondition.notify_all();
				return;
			}

			mInFlightSubmissions.push_back(std::make_pair(&queue, eventValue));
			lock.unlock();
			mSubmissionCondition.notify_all();
		}

		bool MetalGpuQueryPool::TryResolve(bool wait)
		{
			Lock stateLock(mStateMutex);
			for (;;)
			{
				if (mRecordedCommandBuffers > 0)
					return false;

				if (mPendingSubmissions > 0)
				{
					if (!wait)
						return false;

					mSubmissionCondition.wait(stateLock, [this]() { return mPendingSubmissions == 0; });
					continue;
				}

				RemoveCompletedSubmissions();
				if (mInFlightSubmissions.empty())
					break;

				if (!wait)
					return false;

				dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
				MTLSharedEventListener* listener = mGpuDevice.GetSharedEventListener();
				u32 listenerWaitCount = 0;
				TInlineArray<std::pair<MetalGpuQueue*, u64>, 4> fallbackSubmissions;
				for (const auto& entry : mInFlightSubmissions)
				{
					id<MTLSharedEvent> event = entry.first->GetSharedEvent();
					if (event == nil || listener == nil)
						fallbackSubmissions.Add(entry);
					else
					{
						listenerWaitCount++;
						[event notifyListener:listener atValue:entry.second block:^(id<MTLSharedEvent>, uint64_t)
						{
							dispatch_semaphore_signal(semaphore);
						}];
					}
				}

				stateLock.unlock();
				for (u32 waitIndex = 0; waitIndex < listenerWaitCount; waitIndex++)
					dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);

				for (const auto& entry : fallbackSubmissions)
					entry.first->WaitUntilIdle();

				stateLock.lock();
				for (const auto& completed : fallbackSubmissions)
				{
					for (auto it = mInFlightSubmissions.begin(); it != mInFlightSubmissions.end(); ++it)
					{
						if (it->first == completed.first && it->second == completed.second)
						{
							mInFlightSubmissions.erase(it);
							break;
						}
					}
				}
			}

			if (mResultsCached)
				return true;

			std::fill(mResolvedResults.begin(), mResolvedResults.end(), 0);
			if (mSubmissionFailed)
			{
				if (!mSubmissionFailureReported)
				{
					B3D_LOG(Error, LogRenderBackend, "A Metal command buffer containing GPU queries failed; its query results are invalid.");
					mSubmissionFailureReported = true;
				}

				mResultsCached = true;
				return true;
			}

			if (mNextQueryId == 0)
			{
				mResultsCached = true;
				return true;
			}

			if (mQueryType == GpuQueryType::Occlusion && mFirstVisibilityResultSlot != RangeAllocator::kInvalidIndex)
			{
				const u64* contents = (const u64*)[mGpuDevice.GetVisibilityResultBuffer() contents];
				if (contents != nullptr)
					memcpy(mResolvedResults.data(), contents + mFirstVisibilityResultSlot, (size_t)mNextQueryId * sizeof(u64));
			}
			else if (mQueryType == GpuQueryType::Timestamp && mFirstCounterHeapSlot != RangeAllocator::kInvalidIndex)
			{
				NSData* resolved = [mCounterHeap resolveCounterRange:NSMakeRange(mFirstCounterHeapSlot, mNextQueryId)];
				const size_t requiredBytes = (size_t)mNextQueryId * sizeof(MTL4TimestampHeapEntry);
				if (resolved != nil && [resolved length] >= requiredBytes)
				{
					const MTL4TimestampHeapEntry* timestamps = (const MTL4TimestampHeapEntry*)[resolved bytes];
					for (u32 queryIndex = 0; queryIndex < mNextQueryId; queryIndex++)
						mResolvedResults[queryIndex] = timestamps[queryIndex].timestamp;
				}
				else
					B3D_LOG(Warning, LogRenderBackend, "Failed to resolve the Metal timestamp counter heap.");
			}

			mResultsCached = true;
			return true;
		}

		u64 MetalGpuQueryPool::GetQueryResult(GpuQueryId queryId, u32 elementIndex)
		{
			Lock lock(mStateMutex);
			if (elementIndex != 0 || !queryId.IsValid() || queryId.Id >= mNextQueryId || !mResultsCached)
				return 0;

			return mResolvedResults[queryId.Id];
		}

		void MetalGpuQueryPool::ResetAllocation()
		{
			Lock lock(mStateMutex);
			RemoveCompletedSubmissions();
			B3D_ENSURE(mPendingSubmissions == 0 && mInFlightSubmissions.empty() && mRecordedCommandBuffers <= 1);
			if (mFirstCounterHeapSlot != RangeAllocator::kInvalidIndex && mNextQueryId > 0)
				[mCounterHeap invalidateCounterRange:NSMakeRange(mFirstCounterHeapSlot, mNextQueryId)];

			mNextQueryId = 0;
			mResultsCached = false;
			mSubmissionFailed = false;
			mSubmissionFailureReported = false;
			std::fill(mResolvedResults.begin(), mResolvedResults.end(), 0);
		}

		void MetalGpuQueryPool::RemoveCompletedSubmissions()
		{
			for (auto it = mInFlightSubmissions.begin(); it != mInFlightSubmissions.end();)
			{
				id<MTLSharedEvent> event = it->first->GetSharedEvent();
				if (event != nil && (u64)[event signaledValue] >= it->second)
					it = mInFlightSubmissions.erase(it);
				else
					++it;
			}
		}

		bool MetalGpuQueryPool::IsQueryAllocated(GpuQueryId queryId) const
		{
			Lock lock(mStateMutex);
			return queryId.IsValid() && queryId.Id < mNextQueryId;
		}
	} // namespace render
} // namespace b3d
