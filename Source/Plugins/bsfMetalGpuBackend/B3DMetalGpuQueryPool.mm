//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuQueryPool.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuQueue.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	namespace render
	{
		struct MetalGpuQueryPool::Impl
		{
			id<MTLBuffer> VisibilityBuffer = nil;
			id<MTLCounterSampleBuffer> CounterBuffer = nil;
		};

		MetalGpuQueryPool::MetalGpuQueryPool(MetalGpuDevice& gpuDevice, const GpuQueryPoolCreateInformation& createInformation)
			: GpuQueryPool(createInformation)
			, mGpuDevice(gpuDevice)
			, mImpl(B3DMakeUnique<Impl>())
		{
			id<MTLDevice> device = gpuDevice.GetMetalDevice();
			mResolvedResults.resize(mPoolSize);

			if (createInformation.Type == GpuQueryType::Occlusion)
			{
				constexpr u64 kMaximumVisibilityResultOffset = 256ull * 1024ull;
				const u64 lastResultOffset = createInformation.PoolSize > 0
					? ((u64)createInformation.PoolSize - 1ull) * sizeof(u64)
					: 0;
				if (createInformation.PoolSize == 0 || lastResultOffset > kMaximumVisibilityResultOffset)
				{
					B3D_LOG(Error, LogRenderBackend,
						"Metal occlusion query pool size {0} exceeds the Apple GPU visibility-buffer limit.",
						createInformation.PoolSize);
				}
				else if (device != nil)
				{
					const NSUInteger byteSize = (NSUInteger)createInformation.PoolSize * sizeof(u64);
					mImpl->VisibilityBuffer = [device newBufferWithLength:byteSize options:MTLResourceStorageModeShared];
					mSupported = mImpl->VisibilityBuffer != nil;
					if (mImpl->VisibilityBuffer == nil)
						B3D_LOG(Error, LogRenderBackend, "Failed to allocate a Metal visibility query buffer.");
				}
			}
			else if (createInformation.Type == GpuQueryType::Timestamp)
			{
				constexpr u64 kMaximumCounterSampleBufferLength = 32ull * 1024ull;
				const u64 resultBytes = (u64)createInformation.PoolSize * sizeof(MTLCounterResultTimestamp);
				id<MTLCounterSet> timestampSet = gpuDevice.GetTimestampCounterSet();
				if (createInformation.PoolSize == 0 || resultBytes > kMaximumCounterSampleBufferLength)
				{
					B3D_LOG(Error, LogRenderBackend,
						"Metal timestamp query pool size {0} exceeds the Apple GPU 32 KiB counter-buffer limit.",
						createInformation.PoolSize);
				}
				else if (device != nil && timestampSet != nil)
				{
					MTLCounterSampleBufferDescriptor* descriptor = [[MTLCounterSampleBufferDescriptor alloc] init];
					descriptor.counterSet = timestampSet;
					descriptor.storageMode = MTLStorageModeShared;
					descriptor.sampleCount = (NSUInteger)createInformation.PoolSize;

					NSError* error = nil;
					mImpl->CounterBuffer = [device newCounterSampleBufferWithDescriptor:descriptor error:&error];
					mSupported = mImpl->CounterBuffer != nil;
					if (mImpl->CounterBuffer == nil)
					{
						const char* reason = error ? [[error localizedDescription] UTF8String] : "unknown error";
						B3D_LOG(Error, LogRenderBackend, "Failed to create MTLCounterSampleBuffer for timestamp pool: {0}", reason);
					}
#if !__has_feature(objc_arc)
					[descriptor release];
#endif
				}
				else
				{
					B3D_LOG(Warning, LogRenderBackend,
						"Timestamp query pool created on a device without RSC_TIMER_QUERIES; results will remain zero.");
				}
			}
			else
			{
				B3D_LOG(Warning, LogRenderBackend,
					"Pipeline-statistics query pools are unsupported by the Metal backend.");
			}
		}

		MetalGpuQueryPool::~MetalGpuQueryPool()
		{
			if (mImpl)
			{
#if !__has_feature(objc_arc)
				[mImpl->VisibilityBuffer release];
				[mImpl->CounterBuffer release];
#endif
				mImpl->VisibilityBuffer = nil;
				mImpl->CounterBuffer = nil;
			}
		}

		id<MTLBuffer> MetalGpuQueryPool::GetVisibilityBuffer() const
		{
			return mImpl->VisibilityBuffer;
		}

		id<MTLCounterSampleBuffer> MetalGpuQueryPool::GetCounterBuffer() const
		{
			return mImpl->CounterBuffer;
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

				for (auto it = mInFlightSubmissions.begin(); it != mInFlightSubmissions.end();)
				{
					id<MTLSharedEvent> event = it->first->GetSharedEvent();
					if (event != nil && (u64)[event signaledValue] >= it->second)
						it = mInFlightSubmissions.erase(it);
					else
						++it;
				}

				if (mInFlightSubmissions.empty())
					break;
				if (!wait)
					return false;

				dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
				u32 listenerWaitCount = 0;
				TInlineArray<std::pair<MetalGpuQueue*, u64>, 4> fallbackSubmissions;
				for (const auto& entry : mInFlightSubmissions)
				{
					id<MTLSharedEvent> event = entry.first->GetSharedEvent();
					MTLSharedEventListener* listener = entry.first->GetSharedEventListener();
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
#if !__has_feature(objc_arc)
				dispatch_release(semaphore);
#endif
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
					B3D_LOG(Error, LogRenderBackend,
						"A Metal command buffer containing GPU queries failed; its query results are invalid.");
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

			if (mQueryType == GpuQueryType::Occlusion && mImpl->VisibilityBuffer != nil)
			{
				const u64* contents = (const u64*)[mImpl->VisibilityBuffer contents];
				if (contents != nullptr)
					memcpy(mResolvedResults.data(), contents, (size_t)mNextQueryId * sizeof(u64));
			}
			else if (mQueryType == GpuQueryType::Timestamp && mImpl->CounterBuffer != nil)
			{
				@autoreleasepool
				{
					NSData* resolved = [mImpl->CounterBuffer resolveCounterRange:NSMakeRange(0, mNextQueryId)];
					const size_t requiredBytes = (size_t)mNextQueryId * sizeof(MTLCounterResultTimestamp);
					if (resolved != nil && [resolved length] >= requiredBytes)
					{
						const MTLCounterResultTimestamp* timestamps = (const MTLCounterResultTimestamp*)[resolved bytes];
						bool invalidSampleFound = false;
						for (u32 queryIndex = 0; queryIndex < mNextQueryId; queryIndex++)
						{
							if (timestamps[queryIndex].timestamp == MTLCounterErrorValue)
							{
								invalidSampleFound = true;
								continue;
							}

							mResolvedResults[queryIndex] = timestamps[queryIndex].timestamp;
						}

						if (invalidSampleFound)
						{
							std::fill(mResolvedResults.begin(), mResolvedResults.end(), 0);
							B3D_LOG(Warning, LogRenderBackend, "Metal returned an invalid timestamp counter sample.");
						}
					}
				}
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
			B3D_ENSURE(mPendingSubmissions == 0 && mInFlightSubmissions.empty()
				&& mRecordedCommandBuffers <= 1);
			mNextQueryId = 0;
			mResultsCached = false;
			mSubmissionFailed = false;
			mSubmissionFailureReported = false;
			std::fill(mResolvedResults.begin(), mResolvedResults.end(), 0);
		}

		bool MetalGpuQueryPool::IsQueryAllocated(GpuQueryId queryId) const
		{
			Lock lock(mStateMutex);
			return queryId.IsValid() && queryId.Id < mNextQueryId;
		}
	} // namespace render
} // namespace b3d
