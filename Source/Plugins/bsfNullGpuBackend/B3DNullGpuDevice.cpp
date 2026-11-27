//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuDevice.h"
#include "GpuBackend/B3DGpuVertexInputManager.h"
#include "B3DNullGpuQueue.h"
#include "B3DNullGpuCommandBuffer.h"
#include "B3DNullGpuCommandBufferPool.h"
#include "B3DNullGpuBuffer.h"
#include "B3DNullTexture.h"
#include "B3DNullGpuProgram.h"
#include "B3DNullGpuPipelineState.h"
#include "B3DNullGpuParameterSet.h"
#include "B3DNullGpuParameterSetPool.h"
#include "B3DNullGpuPipelineParameterLayout.h"
#include "B3DNullSamplerState.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "B3DNullEventQuery.h"
#include "B3DNullGpuQueryPool.h"
#include "GpuBackend/B3DVideoModeInfo.h"
#include "GpuBackend/B3DGpuTimelineFence.h"
#include "Math/B3DMatrix4.h"
#include "Image/B3DPixelUtility.h"
#include "GpuBackend/Allocators/B3DGpuTransientHeapPool.h"
#include "GpuBackend/Allocators/B3DGpuPagePool.h"
#include "GpuBackend/B3DGpuVirtualMemory.h"
#include "GpuBackend/B3DGpuPageMappingQueue.h"
#include "Utility/B3DBitwise.h"

namespace b3d
{
	namespace render
	{
		namespace
		{
			/**
			 * Always-signaled timeline-fence used by the null backend. There is no GPU work, so any
			 * value the caller asks about is "done". Lets allocator deferred-delete cycles complete
			 * instantly without blocking anything.
			 */
			class NullGpuTimelineFence final : public GpuTimelineFence
			{
			public:
				u64 GetCompletedValue() const final { return ~u64(0); }
			};
		}

		NullGpuDevice::NullGpuDevice()
			: mResourceManager(*this)
		{
			mVideoModeInfo = B3DMakeShared<VideoModeInfo>();

			TGpuTlsfAllocator<NullHeapBackend>::Configuration configuration;
			configuration.InitialHeapSize = 64ull * 1024 * 1024;
			configuration.MaxHeapSize = 256ull * 1024 * 1024;
			configuration.GrowthFactor = 2;
			configuration.MaxEmptyHeapCount = 1;
			configuration.MinAllocationSize = 16;
			configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;

			mPersistentAllocator = B3DMakeUnique<TGpuTlsfAllocator<NullHeapBackend>>(&mHeapBackend, nullptr, configuration);

			TGpuTransientHeapPool<NullHeapBackend>::Configuration transientConfiguration;
			transientConfiguration.CompletionTracker = &mFrameCompletionTracker;
			mTransientHeapPool = AddTransientHeapPool(B3DMakeUnique<TGpuTransientHeapPool<NullHeapBackend>>(&mHeapBackend, transientConfiguration));

			TGpuPagePool<NullHeapBackend>::Configuration pageConfiguration;
			pageConfiguration.CompletionTracker = &mFrameCompletionTracker;
			pageConfiguration.MaxEmptyHeapCount = 1;
			mPagePool = AddPagePool(B3DMakeUnique<TGpuPagePool<NullHeapBackend>>(&mHeapBackend, pageConfiguration));
		}

		NullGpuDevice::~NullGpuDevice()
		{
			if(mSubmitThread != nullptr)
			{
				WaitUntilIdle();
				mPageMappingQueue->Clear();
				ReleaseUnpinnedResources(true);
				DestroyTransferQueue();
				mSubmitThread = nullptr;
			}

			mVertexInputManager = nullptr;
			DestroyTransientHeapPools();
			DestroyPagePools();
		}

		bool NullGpuDevice::Initialize()
		{
			if (mIsInitialized)
				return true;

			// Create a single queue for each type
			for (u32 i = 0; i < GQT_COUNT; i++)
			{
				mQueueInfos[i].FamilyIndex = i;
				mQueueInfos[i].Queues.Add(B3DMakeShared<NullGpuQueue>(*this, (GpuQueueType)i, 0));
			}

			// Initialize capabilities with reasonable defaults for a null backend
			InitializeCapabilities();

			// Submissions go through the submit thread, like on every other device
			IGpuSubmitThreadBackend& submitThreadBackend = *this;
			mSubmitThread = B3DMakeUnique<GpuSubmitThread>(*this, submitThreadBackend);
			CreateTransferQueue();

			mIsInitialized = true;
			return true;
		}

		void NullGpuDevice::WaitUntilIdle()
		{
			// The submit thread lives from the end of initialization to the start of destruction
			if(mSubmitThread != nullptr)
				GetSubmitThread().WaitUntilIdle();
		}

		void NullGpuDevice::ExecuteSubmit(GpuQueue& queue, const TShared<GpuCommandBuffer>& commandBuffer, const GpuQueueFenceValues& queueWaits,
			TArrayView<const GpuTimelineFenceAndValue> waitFences, TArrayView<const GpuTimelineFenceAndValue> signalFences)
		{
			// Executes instantly
			static_cast<NullGpuCommandBuffer&>(*commandBuffer).SetState(GpuCommandBufferState::Done);
			commandBuffer->OnDidComplete();
			commandBuffer->OnDidComplete.Clear();
			commandBuffer->OnDestroyed.Clear();
		}

		void NullGpuDevice::ExecutePageMappings(TArrayView<const GpuPageMapping> mappings, const GpuTimelineFenceAndValue& signalFence)
		{
			// Virtual resources have no memory to map, and null fences are always signalled
		}

		void NullGpuDevice::InitializeCapabilities()
		{
			// Set driver version
			mCapabilities.DriverVersion.Major = 1;
			mCapabilities.DriverVersion.Minor = 0;
			mCapabilities.DriverVersion.Release = 0;
			mCapabilities.DriverVersion.Build = 0;

			mCapabilities.DeviceName = "Null Render Device";
			mCapabilities.DeviceVendor = GPU_UNKNOWN;
			mCapabilities.BackendName = "Null";

			// Support common features for maximum compatibility
			mCapabilities.SetCapability(RSC_COMPUTE_PROGRAM);
			mCapabilities.SetCapability(RSC_GEOMETRY_PROGRAM);
			mCapabilities.SetCapability(RSC_TESSELLATION_PROGRAM);
			mCapabilities.SetCapability(RSC_LOAD_STORE);
			mCapabilities.SetCapability(RSC_LOAD_STORE_MSAA);
			mCapabilities.SetCapability(RSC_TEXTURE_COMPRESSION_BC);
			mCapabilities.SetCapability(RSC_TEXTURE_COMPRESSION_ETC2);
			mCapabilities.SetCapability(RSC_TEXTURE_COMPRESSION_ASTC);
			mCapabilities.SetCapability(RSC_BYTECODE_CACHING);
			mCapabilities.SetCapability(RSC_TEXTURE_VIEWS);
			mCapabilities.SetCapability(RSC_RENDER_TARGET_LAYERS);
			mCapabilities.SetCapability(RSC_MULTI_THREADED_CB);

			// Page tables work as on other devices, without memory behind the pages
			// TODO - Temporary, so the virtual memory core can be tested before other backends support it. Remove once GpuVirtualMemoryTestSuite
			// runs on Vulkan and D3D12 with virtual buffers and textures (end of phase 3 of the virtual memory plan): these capabilities, the page pool, GetPagePool(), TryGetVirtualTextureLayout(), the size 0 resources of
			// virtual buffers and the copy skip in NullGpuCommandBuffer.
			if(gGpuVirtualMemory)
			{
				mCapabilities.SetCapability(RSC_VIRTUAL_BUFFERS);
				mCapabilities.SetCapability(RSC_VIRTUAL_TEXTURES);

				GpuVirtualMemoryCapabilities& virtualMemory = mCapabilities.VirtualMemory;
				virtualMemory.MaximumVirtualSize = 1ull << 40;
				virtualMemory.SupportsVolumeTextures = true;
				virtualMemory.SupportsMipmappedTextureArrays = true;
				virtualMemory.SupportsRenderTargets = true;
				virtualMemory.SupportsPageAliasing = true;
				virtualMemory.ReadsZeroFromUncommittedPages = true;
			}

			// Set conventions (matching Vulkan for consistency)
			mCapabilities.Conventions.NdcYAxis = GpuBackendConventions::Axis::Down;
			mCapabilities.Conventions.MatrixOrder = GpuBackendConventions::MatrixOrder::ColumnMajor;

			// Set reasonable limits
			mCapabilities.VertexBufferCount = 16;
			mCapabilities.RenderTargetCount = 8;

			// Texture units per stage
			constexpr u16 textureUnitsPerStage = 16;
			for (u32 i = 0; i < kGpuRasterComputeProgramTypeCount; i++)
			{
				mCapabilities.SampledTexturesPerStage[i] = textureUnitsPerStage;
				mCapabilities.UniformBufferCountPerStage[i] = 16;
			}

			// Storage texture units (only fragment and compute)
			mCapabilities.StorageTexturesPerStage[GPT_FRAGMENT_PROGRAM] = 8;
			mCapabilities.StorageTexturesPerStage[GPT_COMPUTE_PROGRAM] = 8;

			// Calculate combined totals
			mCapabilities.TotalSampledTexturesCount = textureUnitsPerStage * kGpuRasterComputeProgramTypeCount;
			mCapabilities.TotalUniformBuffersCount = 16 * kGpuRasterComputeProgramTypeCount;
			mCapabilities.TotalStorageTexturesCount = 16;

			mCapabilities.GeometryProgramNumOutputVertices = 1024;
			mCapabilities.MinimumUniformBufferOffsetAlignment = 16;

			// Add shader profile
			mCapabilities.AddShaderProfile(kGpuProgramLanguageNullsl);
		}

		TShared<GpuProgramBytecode> NullGpuDevice::CompileGpuProgramBytecode(const GpuProgramCreateInformation& createInformation) const
		{
			return B3DMakeShared<GpuProgramBytecode>();
		}

		u32 NullGpuDevice::GetQueueCount(GpuQueueType type) const
		{
			return (u32)mQueueInfos[(u32)type].Queues.size();
		}

		TShared<GpuQueue> NullGpuDevice::GetQueue(GpuQueueType type, u32 index) const
		{
			if (index < mQueueInfos[(u32)type].Queues.size())
				return mQueueInfos[(u32)type].Queues[index];

			return nullptr;
		}

		TShared<render::GpuCommandBufferPool> NullGpuDevice::CreateGpuCommandBufferPool(const render::GpuCommandBufferPoolCreateInformation& createInformation)
		{
			return B3DMakeSharedFromExisting(new(B3DAllocate<NullGpuCommandBufferPool>()) NullGpuCommandBufferPool(*this, createInformation));
		}

		IGpuHeap* NullHeapBackend::CreateHeap(u64 sizeInBytes, const HeapCreateInformation& createInformation)
		{
			NullGpuHeap* heap = B3DNew<NullGpuHeap>();
			heap->Size = sizeInBytes;

			return heap;
		}

		void NullHeapBackend::DestroyHeap(HeapHandle handle)
		{
			B3DDelete(static_cast<NullGpuHeap*>(handle));
		}

		GpuMemoryRequirements NullGpuDevice::GetMemoryRequirements(const TextureCreateInformation& createInformation) const
		{
			const TextureProperties properties(createInformation);

			u64 size = 0;
			for(u32 mipLevel = 0; mipLevel <= properties.MipMapCount; mipLevel++)
			{
				const u32 mipWidth = std::max(1u, properties.Width >> mipLevel);
				const u32 mipHeight = std::max(1u, properties.Height >> mipLevel);
				const u32 mipDepth = std::max(1u, properties.Depth >> mipLevel);

				size += (u64)PixelUtility::GetMemorySize(mipWidth, mipHeight, mipDepth, properties.Format) * properties.GetFaceCount();
			}

			GpuMemoryRequirements output;
			output.Size = std::max(size, (u64)1) * std::max(properties.SampleCount, 1u);
			output.Alignment = 256;

			return output;
		}

		GpuMemoryRequirements NullGpuDevice::GetMemoryRequirements(const GpuBufferCreateInformation& createInformation) const
		{
			GpuMemoryRequirements output;
			output.Size = std::max(b3d::GpuBuffer::CalculateTotalBufferSize(createInformation, *this), 1u);
			output.Alignment = 16;

			return output;
		}

		IGpuAllocator& NullGpuDevice::GetPersistentAllocator(u32 memoryType)
		{
			B3D_ASSERT(memoryType == 0);

			return *mPersistentAllocator;
		}

		IGpuTransientHeapPool* NullGpuDevice::GetTransientHeapPool(u32 memoryType)
		{
			return memoryType == 0 ? mTransientHeapPool : nullptr;
		}

		IGpuPagePool* NullGpuDevice::GetPagePool(u32 memoryType)
		{
			return memoryType == 0 ? mPagePool : nullptr;
		}

		bool NullGpuDevice::TryGetVirtualTextureLayout(const TextureCreateInformation& createInformation, GpuVirtualTextureLayout& outLayout) const
		{
			if(!mCapabilities.HasCapability(RSC_VIRTUAL_TEXTURES))
				return false;

			const TextureProperties properties(createInformation);
			const u32 blockSize = PixelUtility::GetBlockSize(properties.Format);
			if(properties.SampleCount > 1 || properties.Type == TEX_TYPE_1D || blockSize == 0 || blockSize > 16 || !Bitwise::IsPow2(blockSize))
				return false;

			// Standard tiles hold 64 KiB of elements (texels, or blocks of compressed formats), and split their bits of size over the axes,
			// larger axes first
			const bool isVolume = properties.Type == TEX_TYPE_3D;
			const u32 axisCount = isVolume ? 3 : 2;
			const u32 sizeBitCount = Bitwise::MostSignificantBit(kGpuPageSize / blockSize);

			i32 tileElementCounts[3] = { 1, 1, 1 };
			for(u32 axis = 0; axis < axisCount; axis++)
			{
				const u32 axisBitCount = sizeBitCount / axisCount + (axis < sizeBitCount % axisCount ? 1 : 0);
				tileElementCounts[axis] = 1 << axisBitCount;
			}

			const Vector2I blockDimensions = PixelUtility::GetBlockDimensions(properties.Format);

			GpuVirtualTextureLayout layout;
			layout.Size = Vector3I((i32)properties.Width, (i32)properties.Height, isVolume ? (i32)properties.Depth : 1);
			layout.MipLevelCount = properties.MipMapCount + 1;
			layout.TileSize = Vector3I(tileElementCounts[0] * blockDimensions.X, tileElementCounts[1] * blockDimensions.Y, tileElementCounts[2]);

			// Mip levels smaller than a tile along any axis share the tail, which packs them tightly
			layout.FirstTailMipLevel = layout.MipLevelCount;
			u64 tailSize = 0;
			for(u32 mipLevel = 0; mipLevel < layout.MipLevelCount; mipLevel++)
			{
				const u32 mipWidth = std::max(properties.Width >> mipLevel, 1u);
				const u32 mipHeight = std::max(properties.Height >> mipLevel, 1u);
				const u32 mipDepth = isVolume ? std::max(properties.Depth >> mipLevel, 1u) : 1;

				const bool isSmallerThanTile = mipWidth < (u32)layout.TileSize.X || mipHeight < (u32)layout.TileSize.Y || mipDepth < (u32)layout.TileSize.Z;
				if(isSmallerThanTile && layout.FirstTailMipLevel == layout.MipLevelCount)
					layout.FirstTailMipLevel = mipLevel;

				if(mipLevel >= layout.FirstTailMipLevel)
					tailSize += PixelUtility::GetMemorySize(mipWidth, mipHeight, mipDepth, properties.Format);
			}

			layout.TailPageCount = (u32)((tailSize + kGpuPageSize - 1) / kGpuPageSize);
			layout.UpdatePageCounts(isVolume ? 1 : properties.GetFaceCount());

			outLayout = layout;
			return true;
		}

		TShared<Texture> NullGpuDevice::CreateTextureInternal(const TextureCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags)
		{
			NullTexture* rawTexture = new(B3DAllocate<NullTexture>()) NullTexture(*this, createInformation, allocation, flags);

			// Default: standalone (calling-thread deletion)
			// With RenderProxy flag: forward destruction to render thread
			TShared<NullTexture> texture = flags.IsSet(GpuObjectCreateFlag::RenderThreadDestroy)
				? B3DMakeSharedFromExisting(rawTexture)
				: MakeSharedStandalone<NullTexture>(rawTexture);

			texture->SetShared(texture);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				texture->Initialize();

			return texture;
		}

		TShared<GpuBuffer> NullGpuDevice::CreateGpuBufferInternal(const GpuBufferCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags)
		{
			NullGpuBuffer* rawBuffer = new(B3DAllocate<NullGpuBuffer>()) NullGpuBuffer(*this, createInformation, allocation);

			// Default: standalone (calling-thread deletion)
			// With RenderProxy flag: forward destruction to render thread
			TShared<NullGpuBuffer> buffer = flags.IsSet(GpuObjectCreateFlag::RenderThreadDestroy)
				? B3DMakeSharedFromExisting(rawBuffer)
				: MakeSharedStandalone<NullGpuBuffer>(rawBuffer);

			buffer->SetShared(buffer);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				buffer->Initialize();

			return buffer;
		}

		GpuVertexInput* NullGpuDevice::CreateVertexInput(u32 id, const GpuVertexInputLayout& layout)
		{
			// Null command buffers record no draws, so they never request vertex inputs
			return nullptr;
		}

		TShared<GpuQueryPool> NullGpuDevice::CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation)
		{
			return B3DMakeShared<NullGpuQueryPool>(*this, createInformation);
		}

		TShared<EventQuery> NullGpuDevice::CreateEventQuery()
		{
			return B3DMakeShared<NullEventQuery>(*this);
		}

		TShared<GpuProgram> NullGpuDevice::CreateGpuProgram(const GpuProgramCreateInformation& createInformation, GpuObjectCreateFlags flags)
		{
			TShared<NullGpuProgram> program = B3DMakeShared<NullGpuProgram>(*this, createInformation);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				program->Initialize();

			return program;
		}

		TShared<GpuGraphicsPipelineState> NullGpuDevice::CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags)
		{
			TShared<NullGpuGraphicsPipelineState> pipelineState = B3DMakeShared<NullGpuGraphicsPipelineState>(*this, createInformation);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				pipelineState->Initialize();

			return pipelineState;
		}

		TShared<GpuComputePipelineState> NullGpuDevice::CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags)
		{
			TShared<NullGpuComputePipelineState> pipelineState = B3DMakeShared<NullGpuComputePipelineState>(*this, createInformation);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				pipelineState->Initialize();

			return pipelineState;
		}

		TShared<GpuPipelineParameterLayout> NullGpuDevice::CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation)
		{
			return B3DMakeShared<NullGpuPipelineParameterLayout>(*this, createInformation);
		}

		class NullGpuPipelineParameterSetLayout : public GpuPipelineParameterSetLayout
		{
		public:
			NullGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription)
				: GpuPipelineParameterSetLayout(parameterDescription)
			{}
		};

		TShared<GpuPipelineParameterSetLayout> NullGpuDevice::CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription, const TShared<GpuResourceTableLayout>& /*resourceTableLayout*/, u32 /*tableIndex*/)
		{
			return B3DMakeShared<NullGpuPipelineParameterSetLayout>(parameterDescription);
		}

		TUnique<GpuParameterSetPool> NullGpuDevice::CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation)
		{
			return B3DMakeUnique<NullGpuParameterSetPool>(*this, createInformation);
		}

		TShared<GpuTimelineFence> NullGpuDevice::CreateTimelineFence()
		{
			return B3DMakeShared<NullGpuTimelineFence>();
		}

		void NullGpuDevice::ConvertProjectionMatrix(const Matrix4& input, Matrix4& output)
		{
			output = input;
		}

		GpuUniformBufferInformation NullGpuDevice::GenerateUniformBufferInformation(const String& name, TArray<GpuUniformBufferMemberInformation>& inOutUniforms)
		{
			GpuUniformBufferInformation bufferInfo;
			bufferInfo.Name = name;
			bufferInfo.Size = 0;
			bufferInfo.IsShareable = true;

			for (auto& uniform : inOutUniforms)
			{
				uniform.GpuOffset = bufferInfo.Size;
				uniform.CpuOffset = bufferInfo.Size;
				bufferInfo.Size += uniform.ElementSize;
			}

			return bufferInfo;
		}

		TShared<SamplerState> NullGpuDevice::CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags)
		{
			TShared<NullSamplerState> samplerState = B3DMakeShared<NullSamplerState>(*this, createInformation);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				samplerState->Initialize();

			return samplerState;
		}
	} // namespace render
} // namespace b3d
