//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuQueue.h"
#include "B3DMetalGpuCommandBuffer.h"
#include "B3DMetalGpuCommandBufferPool.h"
#include "B3DMetalGpuBuffer.h"
#include "B3DMetalTexture.h"
#include "B3DMetalHeapAllocator.h"
#include "B3DMetalResourceManager.h"
#include "B3DMetalClearPipeline.h"
#include "B3DMetalGpuProgram.h"
#include "B3DMetalGpuPipelineState.h"
#include "Math/B3DMath.h"
#include "B3DMetalGpuParameterSet.h"
#include "B3DMetalGpuParameterSetPool.h"
#include "B3DMetalShaderABI.h"
#include "B3DMetalGpuPipelineParameterLayout.h"
#include "Material/B3DShaderCompiler.h"
#include "B3DMetalSamplerState.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DGpuPushConstants.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "B3DMetalEventQuery.h"
#include "B3DMetalGpuQueryPool.h"
#include "MacOS/B3DMacOSVideoModeInfo.h"
#include "GpuBackend/B3DGpuTimelineFence.h"
#include "B3DMetalGpuTimelineFence.h"
#include "Math/B3DMatrix4.h"
#include "Utility/B3DCommonTypes.h"
#include "Debug/B3DLog.h"
#include "Utility/B3DScopeGuard.h"
#include <atomic>
#include <cstring>
#include "Threading/B3DThreading.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "B3DMetalVertexInput.h"
#include "B3DMetalUtility.h"


namespace b3d
{
	namespace render
	{
		MetalGpuDevice::MetalGpuDevice()
		{
			mVideoModeInfo = B3DMakeShared<MacOSVideoModeInfo>();
		}

		MetalGpuDevice::~MetalGpuDevice()
		{
			if (mSubmitThread != nullptr)
			{
				WaitUntilIdle();
				mSubmitThread = nullptr;
			}

			mClearPipeline.reset();

			// Queues the cached vertex inputs for destroy while the resource manager is still alive
			mVertexInputManager = nullptr;
			mResourceManager.reset();

			// The pool creates its heaps through the heap allocator's backend
			DestroyTransientHeapPools();
			if (mHeapAllocator != nullptr)
			{
				mHeapAllocator->Free(mNullVertexBuffer, mNullVertexBufferAllocation);
				mHeapAllocator->Free(mDummyArgumentBuffer, mDummyArgumentBufferAllocation);
				mHeapAllocator->Free(mVisibilityResultBuffer, mVisibilityResultBufferAllocation);
				mHeapAllocator.reset();
			}

			mCounterHeaps.Clear();

			mNullVertexBuffer = nil;
			mDummyArgumentBuffer = nil;
			mVisibilityResultBuffer = nil;

			for (QueueInfo& queueInfo : mQueueInfos)
				queueInfo.Queues.Clear();

			// No blocking wait can still be registered: every fence and query pool is owned by
			// objects torn down above, and the queues have been drained.
			mSharedEventListener = nil;
			mListenerDispatchQueue = nullptr;
			mCompletionListener = nil;
			mCompletionDispatchQueue = nullptr;

			mMetalDevice = nil;
		}

		void MetalGpuDevice::BeginFrame()
		{
			ASSERT_IF_NOT_RENDER_THREAD
		}

		bool MetalGpuDevice::Initialize()
		{
			if (mIsInitialized)
				return true;

			// The engine cannot run without a device, so every failure below is fatal

			// Apple Silicon Macs expose one integrated GPU, so the system default is the only supported adapter.
			mMetalDevice = MTLCreateSystemDefaultDevice();
			if (mMetalDevice == nil)
				B3D_LOG(Fatal, LogRenderBackend, "Failed to acquire a default Metal device. The Metal backend requires a Metal-capable GPU.");

			NSString* deviceName = [mMetalDevice name];
			const String deviceNameString = deviceName ? String([deviceName UTF8String]) : String("<unknown>");
			if (![mMetalDevice supportsFamily:MTLGPUFamilyApple7])
				B3D_LOG(Fatal, LogRenderBackend, "Metal backend requires an Apple Silicon GPU (Apple family 7 or newer). Reported device '{0}' does not qualify.", deviceNameString);

			if (![mMetalDevice supportsFamily:MTLGPUFamilyMetal4])
				B3D_LOG(Fatal, LogRenderBackend, "Metal backend requires Metal 4 support. Reported device '{0}' does not qualify.", deviceNameString);

			if (![mMetalDevice hasUnifiedMemory])
				B3D_LOG(Fatal, LogRenderBackend, "Metal backend requires Apple Silicon unified memory. Reported device '{0}' does not expose unified memory.", deviceNameString);

			// The parameter-set ABI requires Tier 2 argument buffers. Query the feature directly;
			// GPU-family inference is not equivalent (Apple family 6 is the first Tier 2 family).
			if ([mMetalDevice argumentBuffersSupport] != MTLArgumentBuffersTier2)
				B3D_LOG(Fatal, LogRenderBackend, "Metal backend requires Tier 2 argument-buffer support. Reported device '{0}' does not qualify.", deviceNameString);

			// One listener serves every blocking CPU wait in the backend (see GetSharedEventListener).
			// Failure is not fatal: the fence and query-pool wait paths fall back to polling when the
			// listener is nil.
			mListenerDispatchQueue = dispatch_queue_create("b3d.metal.eventlistener", DISPATCH_QUEUE_CONCURRENT);
			if (mListenerDispatchQueue != nullptr)
				mSharedEventListener = [[MTLSharedEventListener alloc] initWithDispatchQueue:mListenerDispatchQueue];

			if (mSharedEventListener == nil)
				B3D_LOG(Warning, LogRenderBackend, "Failed to create the Metal shared event listener; blocking CPU waits will poll instead.");

			mCompletionDispatchQueue = dispatch_queue_create("b3d.metal.completion", DISPATCH_QUEUE_SERIAL);
			if (mCompletionDispatchQueue != nullptr)
				mCompletionListener = [[MTLSharedEventListener alloc] initWithDispatchQueue:mCompletionDispatchQueue];

			if (mCompletionListener == nil)
				B3D_LOG(Fatal, LogRenderBackend, "Failed to create the Metal submission completion listener.");

			mHeapAllocator = B3DMakeUnique<MetalHeapAllocator>(*this);

			// Create one command queue per GpuQueueType. Metal exposes a single unified queue family
			// (every queue accepts graphics, compute, and blit work), so the per-type split mirrors
			// the engine's abstraction without any real affinity.
			for (u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
			{
				MTL4CommandQueueDescriptor* queueDescriptor = [[MTL4CommandQueueDescriptor alloc] init];
				queueDescriptor.label = [NSString stringWithFormat:@"Banshee queue %u", queueTypeIndex];

				NSError* error = nil;
				id<MTL4CommandQueue> commandQueue = [mMetalDevice newMTL4CommandQueueWithDescriptor:queueDescriptor error:&error];
				if (commandQueue == nil)
				{
					B3D_LOG(Fatal, LogRenderBackend, "Failed to create a Metal command queue for queue type {0}: {1}", queueTypeIndex,
						error ? String([[error localizedDescription] UTF8String]) : String("No error details were provided."));
				}

				[commandQueue addResidencySet:mHeapAllocator->GetResidencySet().GetMetalResidencySet()];

				id<MTLSharedEvent> queueEvent = [mMetalDevice newSharedEvent];
				if (queueEvent == nil)
					B3D_LOG(Fatal, LogRenderBackend, "Failed to create a Metal shared event for queue type {0}.", queueTypeIndex);

				mQueueInfos[queueTypeIndex].FamilyIndex = queueTypeIndex;
				mQueueInfos[queueTypeIndex].Queues.Add(B3DMakeShared<MetalGpuQueue>(*this, (GpuQueueType)queueTypeIndex, 0, commandQueue, queueEvent));
			}

			InitializeCapabilities();

			mTransientHeapPool = AddTransientHeapPool(mHeapAllocator->CreateTransientHeapPool(mFrameCompletionTracker));
			mResourceManager = B3DMakeUnique<MetalResourceManager>(*this);

			mClearPipeline = B3DMakeUnique<MetalClearPipeline>(*this);

			const GpuAllocation sharedAllocation = GpuAllocation::CreatePending(mHeapAllocator->GetAllocator(MetalHeapAllocator::kMemoryTypeShared));
			mNullVertexBuffer = mHeapAllocator->AllocateBuffer(kMetalNullVertexStreamStride, MetalHeapAllocator::kMemoryTypeShared, sharedAllocation, mNullVertexBufferAllocation);
			if (mNullVertexBuffer == nil)
				B3D_LOG(Fatal, LogRenderBackend, "Failed to create the shared null vertex buffer.");

			std::memset([mNullVertexBuffer contents], 0, kMetalNullVertexStreamStride);

			// Pools reset their slots before their first use, so the initial contents only matter to queries that were never reset
			const u64 visibilityResultBufferSize = (u64)kVisibilityResultSlotCount * sizeof(u64);
			mVisibilityResultBuffer = mHeapAllocator->AllocateBuffer(visibilityResultBufferSize, MetalHeapAllocator::kMemoryTypeShared, sharedAllocation, mVisibilityResultBufferAllocation);
			if (mVisibilityResultBuffer == nil)
				B3D_LOG(Fatal, LogRenderBackend, "Failed to create the visibility result buffer.");

			std::memset([mVisibilityResultBuffer contents], 0, (size_t)visibilityResultBufferSize);

			constexpr u32 kDummyArgumentBufferSize = 65536;
			mDummyArgumentBuffer = mHeapAllocator->AllocateBuffer(kDummyArgumentBufferSize, MetalHeapAllocator::kMemoryTypeShared, sharedAllocation, mDummyArgumentBufferAllocation);
			if (mDummyArgumentBuffer == nil)
				B3D_LOG(Fatal, LogRenderBackend, "Failed to create the shared dummy argument buffer.");

			std::memset([mDummyArgumentBuffer contents], 0, kDummyArgumentBufferSize);

			IGpuSubmitThreadBackend& submitThreadBackend = *this;
			mSubmitThread = B3DMakeUnique<GpuSubmitThread>(*this, submitThreadBackend);

			mIsInitialized = true;
			return true;
		}

		void MetalGpuDevice::InitializeCapabilities()
		{
			// Metal does not expose a driver version
			mCapabilities.DriverVersion.Major = 0;
			mCapabilities.DriverVersion.Minor = 0;
			mCapabilities.DriverVersion.Release = 0;
			mCapabilities.DriverVersion.Build = 0;

			NSString* deviceName = [mMetalDevice name];
			mCapabilities.DeviceName = deviceName ? String([deviceName UTF8String]) : String();
			mCapabilities.DeviceVendor = GPU_APPLE;
			mCapabilities.BackendName = "Metal";

			B3D_ASSERT([mMetalDevice supportsFamily:MTLGPUFamilyApple7] && [mMetalDevice supportsFamily:MTLGPUFamilyMetal4]);

			// Metal has no geometry shaders. Tessellation stays unadvertised so tessellation shaders are rejected by the engine
			// instead of failing Metal validation at pipeline creation.
			// TODO - Support tessellation (Metal runs it as a compute pre-pass feeding a post-tessellation vertex stage)
			mCapabilities.SetCapability(RSC_COMPUTE_PROGRAM);
			mCapabilities.SetCapability(RSC_LOAD_STORE);
			mCapabilities.SetCapability(RSC_LOAD_STORE_MSAA);

			if ([mMetalDevice supportsBCTextureCompression])
				mCapabilities.SetCapability(RSC_TEXTURE_COMPRESSION_BC);
			mCapabilities.SetCapability(RSC_TEXTURE_COMPRESSION_ETC2);
			mCapabilities.SetCapability(RSC_TEXTURE_COMPRESSION_ASTC);
			mCapabilities.SetCapability(RSC_BYTECODE_CACHING);
			mCapabilities.SetCapability(RSC_TEXTURE_VIEWS);
			mCapabilities.SetCapability(RSC_RENDER_TARGET_LAYERS);
			mCapabilities.SetCapability(RSC_MULTI_THREADED_CB);

			// Metal 4 writes timestamps at any point of a command buffer, inside or outside of encoders
			mTimestampFrequency = [mMetalDevice queryTimestampFrequency];
			if (mTimestampFrequency != 0)
				mCapabilities.SetCapability(RSC_TIMER_QUERIES);

			// Metal clip space is y-up like D3D, so the MSL compiler disables SPIRV-Cross's flip_vert_y
			mCapabilities.Conventions.NdcYAxis = GpuBackendConventions::Axis::Up;
			mCapabilities.Conventions.MatrixOrder = GpuBackendConventions::MatrixOrder::ColumnMajor;

			// Metal has 31 vertex-stage buffer slots: parameter-set argument buffers use 0..7, dynamic-offset uniform buffers
			// 8..15, vertex streams 16..29 and push constants 30
			static_assert(kMetalDynamicUniformBufferIndexBase + kMetalDynamicUniformBufferCount == kMetalVertexBufferSlotBase,
				"The Metal vertex-stream range must immediately follow the dynamic uniform-buffer range.");
			static_assert(kMetalVertexBufferSlotEnd == kMetalPushConstantBufferIndex,
				"The Metal push-constant buffer must immediately follow the vertex-stream range.");
			mCapabilities.VertexBufferCount = kMetalVertexBufferSlotEnd - kMetalVertexBufferSlotBase;
			mCapabilities.MaximumPushConstantSize = kMaxPushConstantSizeInBytes;
			mCapabilities.RenderTargetCount = 8;

			constexpr u16 resourcesPerStage = (kMetalMaximumParameterSetIndex + 1) * (kMetalMaximumArgumentBufferSlot + 1);
			const GpuProgramType supportedStages[] =
			{
				GPT_VERTEX_PROGRAM,
				GPT_FRAGMENT_PROGRAM,
				GPT_COMPUTE_PROGRAM
			};

			for (GpuProgramType stage : supportedStages)
			{
				mCapabilities.SampledTexturesPerStage[stage] = resourcesPerStage;
				mCapabilities.UniformBufferCountPerStage[stage] = resourcesPerStage;
				mCapabilities.StorageTexturesPerStage[stage] = resourcesPerStage;
			}

			mCapabilities.TotalSampledTexturesCount = resourcesPerStage * 3;
			mCapabilities.TotalUniformBuffersCount = resourcesPerStage * 3;
			mCapabilities.TotalStorageTexturesCount = resourcesPerStage * 3;

			// Parameter-set updates validate this same alignment before encoding buffer addresses
			mCapabilities.MinimumUniformBufferOffsetAlignment = 16;

			mCapabilities.AddShaderProfile(kGpuProgramLanguageName);
		}

		u32 MetalGpuDevice::GetQueueCount(GpuQueueType type) const
		{
			return (u32)mQueueInfos[(u32)type].Queues.size();
		}

		TShared<GpuQueue> MetalGpuDevice::GetQueue(GpuQueueType type, u32 index) const
		{
			if (index < mQueueInfos[(u32)type].Queues.size())
				return mQueueInfos[(u32)type].Queues[index];

			return nullptr;
		}

		TShared<render::GpuCommandBufferPool> MetalGpuDevice::CreateGpuCommandBufferPool(const render::GpuCommandBufferPoolCreateInformation& createInformation)
		{
			return B3DMakeSharedFromExisting(new(B3DAllocate<MetalGpuCommandBufferPool>()) MetalGpuCommandBufferPool(*this, createInformation));
		}

		GpuMemoryRequirements MetalGpuDevice::GetMemoryRequirements(const TextureCreateInformation& createInformation) const
		{
			MTLTextureDescriptor* descriptor = MetalTexture::CreateDescriptor(GetMetalDevice(), TextureProperties(createInformation));
			if (descriptor == nil)
			{
				GpuMemoryRequirements output;
				output.MemoryType = GpuMemoryRequirements::kUnsupportedMemoryType;
				return output;
			}

			const GpuMemoryRequirements output = mHeapAllocator->GetTextureMemoryRequirements(descriptor);
			return output;
		}

		GpuMemoryRequirements MetalGpuDevice::GetMemoryRequirements(const GpuBufferCreateInformation& createInformation) const
		{
			// Metal disallows zero-length buffers; clamp to a small minimum
			u64 size = b3d::GpuBuffer::CalculateTotalBufferSize(createInformation, *this);
			if (size == 0)
				size = 64;

			return mHeapAllocator->GetBufferMemoryRequirements(size, MetalHeapAllocator::GetBufferMemoryType(createInformation));
		}

		IGpuAllocator& MetalGpuDevice::GetPersistentAllocator(u32 memoryType)
		{
			return mHeapAllocator->GetAllocator(memoryType);
		}

		TShared<Texture> MetalGpuDevice::CreateTextureInternal(const TextureCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags)
		{
			MetalTexture* rawTexture = new(B3DAllocate<MetalTexture>()) MetalTexture(*this, createInformation, allocation, flags);

			TShared<MetalTexture> texture = flags.IsSet(GpuObjectCreateFlag::RenderThreadDestroy)
				? B3DMakeSharedFromExisting(rawTexture)
				: MakeSharedStandalone<MetalTexture>(rawTexture);

			texture->SetShared(texture);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				texture->Initialize();

			return texture;
		}

		TShared<GpuBuffer> MetalGpuDevice::CreateGpuBufferInternal(const GpuBufferCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags)
		{
			MetalGpuBuffer* rawBuffer = new(B3DAllocate<MetalGpuBuffer>()) MetalGpuBuffer(*this, createInformation, allocation);

			TShared<MetalGpuBuffer> buffer = flags.IsSet(GpuObjectCreateFlag::RenderThreadDestroy)
				? B3DMakeSharedFromExisting(rawBuffer)
				: MakeSharedStandalone<MetalGpuBuffer>(rawBuffer);

			buffer->SetShared(buffer);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				buffer->Initialize();

			return buffer;
		}

		GpuVertexInput* MetalGpuDevice::CreateVertexInput(u32 id, const GpuVertexInputLayout& layout)
		{
			// Validate that the resolved layout is expressible on Metal before allocating anything.
			if (kMetalVertexBufferSlotBase + layout.StreamCount > kMetalVertexBufferSlotEnd)
			{
				B3D_LOG(Error, LogRenderBackend, "Vertex input requires {0} streams, exceeding the {1} vertex-stage buffer slots available on Metal.",
					layout.StreamCount, kMetalVertexBufferSlotEnd - kMetalVertexBufferSlotBase);
				return nullptr;
			}

			for (const GpuVertexInputAttribute& attribute : layout.Attributes)
			{
				const VertexElementType elementType = attribute.BufferElement != nullptr
					? attribute.BufferElement->GetType()
					: attribute.ShaderInput->GetType();
				if (MetalUtility::GetVertexFormat(elementType) == MTLVertexFormatInvalid)
				{
					B3D_LOG(Error, LogRenderBackend, "Unsupported vertex element type {0} on the Metal backend.", (u32)elementType);
					return nullptr;
				}

				if (attribute.ShaderInput->GetOffset() > kMetalMaxVertexAttributeIndex)
				{
					B3D_LOG(Error, LogRenderBackend, "Vertex shader input location {0} exceeds Metal's maximum attribute index {1}.",
						attribute.ShaderInput->GetOffset(), kMetalMaxVertexAttributeIndex);
					return nullptr;
				}
			}

			return GetResourceManager().Create<MetalVertexInput>(id, layout);
		}

		TUnique<IGpuAllocator> MetalGpuDevice::CreateScratchAllocator(u32 memoryType,
			IGpuCompletionTracker& completionTracker)
		{
			if (mHeapAllocator == nullptr)
				return nullptr;

			return mHeapAllocator->CreateScratchAllocator(memoryType, completionTracker);
		}

		IGpuTransientHeapPool* MetalGpuDevice::GetTransientHeapPool(u32 memoryType)
		{
			return memoryType == MetalHeapAllocator::kMemoryTypePrivate ? mTransientHeapPool : nullptr;
		}

		TShared<GpuQueryPool> MetalGpuDevice::CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation)
		{
			// Without timer queries the device cannot write timestamps at arbitrary points of a command buffer
			if (createInformation.Type == GpuQueryType::Timestamp && !mCapabilities.HasCapability(RSC_TIMER_QUERIES))
				return nullptr;

			TShared<MetalGpuQueryPool> queryPool = B3DMakeShared<MetalGpuQueryPool>(*this, createInformation);

			// Visibility results and timestamps are written to slots shared with the device's other pools, which are limited
			if (createInformation.Type == GpuQueryType::Occlusion && queryPool->GetVisibilityResultRange().length == 0)
				return nullptr;

			if (createInformation.Type == GpuQueryType::Timestamp && queryPool->GetCounterHeap() == nil)
				return nullptr;

			return queryPool;
		}

		TShared<EventQuery> MetalGpuDevice::CreateEventQuery()
		{
			return B3DMakeShared<MetalEventQuery>(*this);
		}

		TShared<GpuProgram> MetalGpuDevice::CreateGpuProgram(const GpuProgramCreateInformation& createInformation, GpuObjectCreateFlags flags)
		{
			TShared<MetalGpuProgram> program = B3DMakeShared<MetalGpuProgram>(*this, createInformation);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				program->Initialize();

			return program;
		}

		TShared<GpuGraphicsPipelineState> MetalGpuDevice::CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags)
		{
			TShared<MetalGpuGraphicsPipelineState> pipelineState = B3DMakeShared<MetalGpuGraphicsPipelineState>(*this, createInformation);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				pipelineState->Initialize();

			return pipelineState;
		}

		TShared<GpuComputePipelineState> MetalGpuDevice::CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags)
		{
			TShared<MetalGpuComputePipelineState> pipelineState = B3DMakeShared<MetalGpuComputePipelineState>(*this, createInformation);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				pipelineState->Initialize();

			return pipelineState;
		}

		TShared<GpuPipelineParameterLayout> MetalGpuDevice::CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation)
		{
			return B3DMakeShared<MetalGpuPipelineParameterLayout>(*this, createInformation);
		}

		TShared<GpuPipelineParameterSetLayout> MetalGpuDevice::CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription, const TShared<GpuResourceTableLayout>& resourceTableLayout, u32 tableIndex)
		{
			return B3DMakeShared<MetalGpuPipelineParameterSetLayout>(parameterDescription, resourceTableLayout,
				tableIndex);
		}

		TUnique<GpuParameterSetPool> MetalGpuDevice::CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation)
		{
			return B3DMakeUnique<MetalGpuParameterSetPool>(*this, createInformation);
		}

		TShared<GpuTimelineFence> MetalGpuDevice::CreateTimelineFence()
		{
			return B3DMakeShared<MetalGpuTimelineFence>(*this);
		}

		void MetalGpuDevice::ConvertProjectionMatrix(const Matrix4& input, Matrix4& output)
		{
			// Metal clip space matches D3D (y-up, z in [0, 1])
			output = input;
		}

		GpuUniformBufferInformation MetalGpuDevice::GenerateUniformBufferInformation(const String& name, TArray<GpuUniformBufferMemberInformation>& inOutUniforms)
		{
			// MSL is generated from SPIR-V with std140 uniform layout, so packing matches VulkanGpuDevice
			GpuUniformBufferInformation bufferInformation;
			bufferInformation.Size = 0;
			bufferInformation.IsShareable = true;
			bufferInformation.Name = name;
			bufferInformation.Slot = 0;
			bufferInformation.Set = 0;

			for (auto& member : inOutUniforms)
			{
				u32 size;
				if (member.Type == GPDT_STRUCT)
				{
					// std140 aligns structs to 16 bytes and pads their size to a multiple of 16
					size = Math::DivideAndRoundUp(member.ElementSize, 16u) * 4;
					bufferInformation.Size = Math::DivideAndRoundUp(bufferInformation.Size, 4u) * 4;
				}
				else
					size = GpuBackendUtility::CalcStd140MemberSizeAndOffset(member.Type, member.ArraySize, bufferInformation.Size);

				member.ElementSize = size;
				member.ArrayElementStride = size;
				member.CpuOffset = bufferInformation.Size;
				member.GpuOffset = 0;
				bufferInformation.Size += size * member.ArraySize;
				member.ParentUniformBufferSlot = 0;
				member.ParentUniformBufferSet = 0;
			}

			if (bufferInformation.Size % 4 != 0)
				bufferInformation.Size += (4 - (bufferInformation.Size % 4));

			return bufferInformation;
		}

		float MetalGpuDevice::ConvertTimestampToMilliseconds(u64 timestamp)
		{
			if (mTimestampFrequency == 0)
				return 0.0f;

			return (float)((double)timestamp * 1000.0 / (double)mTimestampFrequency);
		}

		void MetalGpuDevice::WaitUntilIdle()
		{
			// The submit thread only exists from the end of Initialize() to the start of destruction
			if (mSubmitThread == nullptr)
			{
				ExecuteWaitUntilIdle();
				return;
			}

			GetSubmitThread().WaitUntilIdle();
		}

		void MetalGpuDevice::ExecuteSubmit(GpuQueue& queue, const TShared<GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences)
		{
			MetalGpuQueue& metalQueue = static_cast<MetalGpuQueue&>(queue);
			MetalGpuCommandBuffer& metalCommandBuffer = static_cast<MetalGpuCommandBuffer&>(*commandBuffer);
			metalCommandBuffer.ExecuteSubmitOnSubmitThread(metalQueue, syncMask, signalFences);
		}

		void MetalGpuDevice::RefreshCompletionState(GpuQueue& queue, bool forceWait, u64 lastFenceValue)
		{
			static_cast<MetalGpuQueue&>(queue).RefreshCompletionState(forceWait, lastFenceValue);
		}

		u64 MetalGpuDevice::GetLastSubmittedFenceValue(const GpuQueue& queue) const
		{
			return static_cast<const MetalGpuQueue&>(queue).GetLastCommittedEventValue();
		}

		void MetalGpuDevice::ExecuteWaitUntilIdle()
		{
			for (u32 typeIndex = 0; typeIndex < GQT_COUNT; typeIndex++)
			{
				const GpuQueueType queueType = (GpuQueueType)typeIndex;
				const u32 queueCount = GetQueueCount(queueType);
				for (u32 queueIndex = 0; queueIndex < queueCount; queueIndex++)
				{
					TShared<GpuQueue> queue = GetQueue(queueType, queueIndex);
					if (queue)
						static_cast<MetalGpuQueue&>(*queue).ExecuteWaitUntilIdle();
				}
			}
		}

		void MetalGpuDevice::ExecuteWaitUntilIdle(GpuQueue& queue)
		{
			static_cast<MetalGpuQueue&>(queue).ExecuteWaitUntilIdle();
		}

		u32 MetalGpuDevice::AllocateVisibilityResultSlots(u32 count)
		{
			Lock lock(mQueryAllocationMutex);
			return mVisibilityResultSlots.Allocate(count);
		}

		void MetalGpuDevice::FreeVisibilityResultSlots(u32 firstSlot, u32 count)
		{
			Lock lock(mQueryAllocationMutex);
			mVisibilityResultSlots.Free(firstSlot, count);
		}

		u32 MetalGpuDevice::AllocateCounterHeapSlots(u32 count, __strong id<MTL4CounterHeap>& outCounterHeap)
		{
			if (!B3D_ENSURE_LOG(count > 0 && count <= kCounterHeapSlotCount, "Cannot reserve {0} timestamp counter heap slots, a heap holds at most {1}.", count, kCounterHeapSlotCount))
				return RangeAllocator::kInvalidIndex;

			Lock lock(mQueryAllocationMutex);
			for (CounterHeap& counterHeap : mCounterHeaps)
			{
				const u32 firstSlot = counterHeap.Slots.Allocate(count);
				if (firstSlot != RangeAllocator::kInvalidIndex)
				{
					outCounterHeap = counterHeap.Heap;
					return firstSlot;
				}
			}

			MTL4CounterHeapDescriptor* descriptor = [[MTL4CounterHeapDescriptor alloc] init];
			descriptor.type = MTL4CounterHeapTypeTimestamp;
			descriptor.count = kCounterHeapSlotCount;

			NSError* error = nil;
			id<MTL4CounterHeap> heap = [mMetalDevice newCounterHeapWithDescriptor:descriptor error:&error];
			if (heap == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Failed to create a timestamp counter heap: {0}",
					error ? String([[error localizedDescription] UTF8String]) : String("No error details were provided."));
				return RangeAllocator::kInvalidIndex;
			}

			mCounterHeaps.Add(CounterHeap());
			CounterHeap& counterHeap = mCounterHeaps.Back();
			counterHeap.Heap = heap;

			outCounterHeap = heap;
			return counterHeap.Slots.Allocate(count);
		}

		void MetalGpuDevice::FreeCounterHeapSlots(id<MTL4CounterHeap> counterHeap, u32 firstSlot, u32 count)
		{
			Lock lock(mQueryAllocationMutex);

			auto iterator = std::find_if(mCounterHeaps.Begin(), mCounterHeaps.End(), [counterHeap](const CounterHeap& entry) { return entry.Heap == counterHeap; });
			if (!B3D_ENSURE(iterator != mCounterHeaps.End()))
				return;

			iterator->Slots.Free(firstSlot, count);
		}

		TShared<SamplerState> MetalGpuDevice::CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags)
		{
			TShared<MetalSamplerState> samplerState = B3DMakeShared<MetalSamplerState>(*this, createInformation);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				samplerState->Initialize();

			return samplerState;
		}
	} // namespace render
} // namespace b3d
