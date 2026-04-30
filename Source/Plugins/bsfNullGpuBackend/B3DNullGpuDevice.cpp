//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuDevice.h"
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
#include "GpuBackend/B3DGpuTransferBufferHelper.h"
#include "GpuBackend/B3DGpuTimelineFence.h"
#include "Math/B3DMatrix4.h"

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
		{
			mVideoModeInfo = B3DMakeShared<VideoModeInfo>();
		}

		NullGpuDevice::~NullGpuDevice()
		{
			mTransferBufferHelper.reset();
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

			mDefaultSubmissionFence = B3DMakeShared<NullGpuTimelineFence>();

			mTransferBufferHelper = B3DMakeUnique<GpuTransferBufferHelper>(*this, GpuQueueId(GQT_GRAPHICS, 0));

			mIsInitialized = true;
			return true;
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

			// Set conventions (matching Vulkan for consistency)
			mCapabilities.Conventions.NdcYAxis = GpuBackendConventions::Axis::Down;
			mCapabilities.Conventions.MatrixOrder = GpuBackendConventions::MatrixOrder::ColumnMajor;

			// Set reasonable limits
			mCapabilities.VertexBufferCount = 16;
			mCapabilities.RenderTargetCount = 8;

			// Texture units per stage
			constexpr u16 textureUnitsPerStage = 16;
			for (u32 i = 0; i < GPT_COUNT; i++)
			{
				mCapabilities.SampledTexturesPerStage[i] = textureUnitsPerStage;
				mCapabilities.UniformBufferCountPerStage[i] = 16;
			}

			// Storage texture units (only fragment and compute)
			mCapabilities.StorageTexturesPerStage[GPT_FRAGMENT_PROGRAM] = 8;
			mCapabilities.StorageTexturesPerStage[GPT_COMPUTE_PROGRAM] = 8;

			// Calculate combined totals
			mCapabilities.TotalSampledTexturesCount = textureUnitsPerStage * GPT_COUNT;
			mCapabilities.TotalUniformBuffersCount = 16 * GPT_COUNT;
			mCapabilities.TotalStorageTexturesCount = 16;

			mCapabilities.GeometryProgramNumOutputVertices = 1024;
			mCapabilities.MinimumUniformBufferOffsetAlignment = 16;

			// Add shader profile
			mCapabilities.AddShaderProfile("nullsl");
		}

		SPtr<GpuProgramBytecode> NullGpuDevice::CompileGpuProgramBytecode(const GpuProgramCreateInformation& createInformation) const
		{
			return B3DMakeShared<GpuProgramBytecode>();
		}

		u32 NullGpuDevice::GetQueueCount(GpuQueueType type) const
		{
			return (u32)mQueueInfos[(u32)type].Queues.size();
		}

		SPtr<GpuQueue> NullGpuDevice::GetQueue(GpuQueueType type, u32 index) const
		{
			if (index < mQueueInfos[(u32)type].Queues.size())
				return mQueueInfos[(u32)type].Queues[index];

			return nullptr;
		}

		SPtr<render::GpuCommandBufferPool> NullGpuDevice::CreateGpuCommandBufferPool(const render::GpuCommandBufferPoolCreateInformation& createInformation)
		{
			return B3DMakeSharedFromExisting(new(B3DAllocate<NullGpuCommandBufferPool>()) NullGpuCommandBufferPool(*this, createInformation));
		}

		SPtr<Texture> NullGpuDevice::CreateTexture(const TextureCreateInformation& createInformation, GpuObjectCreateFlags flags)
		{
			NullTexture* rawTexture = new(B3DAllocate<NullTexture>()) NullTexture(*this, createInformation);

			// Default: standalone (calling-thread deletion)
			// With RenderProxy flag: forward destruction to render thread
			SPtr<NullTexture> texture = flags.IsSet(GpuObjectCreateFlag::RenderThreadDestroy)
				? B3DMakeSharedFromExisting(rawTexture)
				: MakeSharedStandalone<NullTexture>(rawTexture);

			texture->SetShared(texture);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				texture->Initialize();

			return texture;
		}

		SPtr<GpuBuffer> NullGpuDevice::CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, GpuObjectCreateFlags flags)
		{
			NullGpuBuffer* rawBuffer = new(B3DAllocate<NullGpuBuffer>()) NullGpuBuffer(*this, createInformation);

			// Default: standalone (calling-thread deletion)
			// With RenderProxy flag: forward destruction to render thread
			SPtr<NullGpuBuffer> buffer = flags.IsSet(GpuObjectCreateFlag::RenderThreadDestroy)
				? B3DMakeSharedFromExisting(rawBuffer)
				: MakeSharedStandalone<NullGpuBuffer>(rawBuffer);

			buffer->SetShared(buffer);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				buffer->Initialize();

			return buffer;
		}

		SPtr<GpuQueryPool> NullGpuDevice::CreateQueryPool(const GpuQueryPoolCreateInformation& createInformation)
		{
			return B3DMakeShared<NullGpuQueryPool>(*this, createInformation);
		}

		SPtr<EventQuery> NullGpuDevice::CreateEventQuery()
		{
			return B3DMakeShared<NullEventQuery>(*this);
		}

		SPtr<GpuProgram> NullGpuDevice::CreateGpuProgram(const GpuProgramCreateInformation& createInformation, GpuObjectCreateFlags flags)
		{
			SPtr<NullGpuProgram> program = B3DMakeShared<NullGpuProgram>(*this, createInformation);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				program->Initialize();

			return program;
		}

		SPtr<GpuGraphicsPipelineState> NullGpuDevice::CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags)
		{
			SPtr<NullGpuGraphicsPipelineState> pipelineState = B3DMakeShared<NullGpuGraphicsPipelineState>(*this, createInformation);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				pipelineState->Initialize();

			return pipelineState;
		}

		SPtr<GpuComputePipelineState> NullGpuDevice::CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags)
		{
			SPtr<NullGpuComputePipelineState> pipelineState = B3DMakeShared<NullGpuComputePipelineState>(*this, createInformation);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				pipelineState->Initialize();

			return pipelineState;
		}

		SPtr<GpuPipelineParameterLayout> NullGpuDevice::CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation)
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

		SPtr<GpuPipelineParameterSetLayout> NullGpuDevice::CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription)
		{
			return B3DMakeShared<NullGpuPipelineParameterSetLayout>(parameterDescription);
		}

		UPtr<GpuParameterSetPool> NullGpuDevice::CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation)
		{
			return B3DMakeUnique<NullGpuParameterSetPool>(*this, createInformation);
		}

		SPtr<GpuTimelineFence> NullGpuDevice::CreateTimelineFence()
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

		SPtr<SamplerState> NullGpuDevice::CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags)
		{
			SPtr<NullSamplerState> samplerState = B3DMakeShared<NullSamplerState>(*this, createInformation);

			if (!flags.IsSet(GpuObjectCreateFlag::DeferredInitialize))
				samplerState->Initialize();

			return samplerState;
		}
	} // namespace render
} // namespace b3d
