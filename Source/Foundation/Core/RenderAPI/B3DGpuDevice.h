//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGpuParameterSetPool.h"
#include "B3DGpuQueries.h"
#include "B3DPrerequisites.h"
#include "B3DSamplerState.h"
#include "B3DGpuTransferBufferHelper.h"

namespace b3d::render
{
	class GpuQueryPool;
	class GpuCommandBufferPoolRing;
	class GpuBuffer;
	class Texture;
}

namespace b3d
{
	class GpuPipelineParameterSetLayout;
	class GpuFrameCapture;
	struct SamplerStateCreateInformation;
	struct TextureCreateInformation;
	struct TextureCopyInformation;
	struct TextureBlitInformation;

	namespace render
	{
		struct GpuCommandBufferPoolCreateInformation;
		class GpuCommandBufferPool;
	}

	struct GpuPipelineParameterLayoutCreateInformation;
	struct GpuProgramBytecode;
	struct GpuBufferCreateInformation;
	struct GpuProgramCreateInformation;
	struct GpuComputePipelineStateCreateInformation;
	struct GpuGraphicsPipelineStateCreateInformation;

	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Flags that determine how is a resource being accessed by the GPU. */
	enum class GpuAccessFlag
	{
		None = 0,
		Read = 0x1,
		Write = 0x2
	};

	typedef Flags<GpuAccessFlag> GpuAccessFlags;
	B3D_FLAGS_OPERATORS(GpuAccessFlag);

	/** Uniquely represents a GPU queue. */
	struct GpuQueueId
	{
		GpuQueueId(u32 id = 0)
			: Id(id)
		{
			B3D_ASSERT(Id < (B3D_MAX_QUEUES_PER_TYPE * GQT_COUNT));
		}

		GpuQueueId(GpuQueueType type, u32 index)
		{
			switch(type)
			{
			case GQT_COMPUTE:
				Id = B3D_MAX_QUEUES_PER_TYPE + index;
				break;
			case GQT_TRANSFER:
				Id = B3D_MAX_QUEUES_PER_TYPE * 2 + index;
				break;
			default:
				Id = index;
			}

			B3D_ASSERT(Id < (B3D_MAX_QUEUES_PER_TYPE * GQT_COUNT));
		}

		GpuQueueType GetType() const
		{
			if(Id >= B3D_MAX_QUEUES_PER_TYPE * 2)
				return GQT_TRANSFER;

			if(Id >= B3D_MAX_QUEUES_PER_TYPE)
				return GQT_COMPUTE;

			return GQT_GRAPHICS;

		}

		u32 GetIndex() const
		{
			if(Id >= B3D_MAX_QUEUES_PER_TYPE * 2)
				return Id - B3D_MAX_QUEUES_PER_TYPE * 2;

			if(Id >= B3D_MAX_QUEUES_PER_TYPE)
				return Id - B3D_MAX_QUEUES_PER_TYPE;

			return Id;
		}

		u32 Id;
	};

	/** Mask that represents zero or multiple GPU queues. */
	struct B3D_EXPORT GpuQueueMask
	{
		GpuQueueMask(u32 mask = 0)
			: Mask(mask)
		{ }

		GpuQueueMask(GpuQueueId id)
		{
			u32 bitShift = 0;
			switch(id.GetType())
			{
			case GQT_GRAPHICS:
				break;
			case GQT_COMPUTE:
				bitShift = 8;
				break;
			case GQT_TRANSFER:
				bitShift = 16;
				break;
			default:
				break;
			}

			Mask = 1 << id.GetIndex() << bitShift;
		}

		bool operator==(GpuQueueMask rhs) const { return Mask == rhs.Mask; }
		bool operator!=(GpuQueueMask rhs) const { return Mask != rhs.Mask; }

		GpuQueueMask& operator=(GpuQueueId id)
		{
			Mask = GpuQueueMask(id).Mask;
			return *this;
		}

		GpuQueueMask& operator|=(GpuQueueMask rhs)
		{
			Mask |= rhs.Mask;
			return *this;
		}

		GpuQueueMask operator|(GpuQueueMask rhs) const
		{
			GpuQueueMask out(*this);
			out |= rhs;

			return out;
		}

		GpuQueueMask& operator&=(GpuQueueMask rhs)
		{
			Mask &= rhs.Mask;
			return *this;
		}

		GpuQueueMask operator&(GpuQueueMask rhs) const
		{
			GpuQueueMask out(*this);
			out &= rhs;

			return out;
		}

		GpuQueueMask& operator^=(GpuQueueMask rhs)
		{
			Mask ^= rhs.Mask;
			return *this;
		}

		GpuQueueMask operator^(GpuQueueMask rhs) const
		{
			GpuQueueMask out(*this);
			out ^= rhs;

			return out;
		}

		GpuQueueMask operator~() const
		{
			GpuQueueMask out;
			out.Mask = ~Mask;

			return out;
		}

		/** Returns true if no queues are part of the mask. */
		bool IsEmpty() const { return Mask == 0; }

		/** Returns true if the queue ID is part of the mask. */
		bool IsSet(GpuQueueId queueId) const { return (Mask & GpuQueueMask(queueId).Mask) != 0; }

		u32 Mask = 0;

		static const GpuQueueMask kNone;
		static const GpuQueueMask kAll;
	};

	/**
	 * Specifies a queue on which command buffers can be submitted on.
	 *
	 * @note	Thread safe.
	 */
	class B3D_EXPORT GpuQueue
	{
	public:
		virtual ~GpuQueue() = default;

		/** Determines which type of command buffer commands can be used on the command buffers submitted on the queue. */
		GpuQueueType GetType() const { return mType; }

		/** Returns the unique index of the queue, for its type. */
		u32 GetIndex() const { return mIndex; }

		/** Returns a unique identifier for this queue. */
		GpuQueueId GetId() const { return GpuQueueId(mType, mIndex); }

		/**
		 * Submits the command buffer for execution on the specified queue.
		 *
		 * @param	commandBuffer	Command buffer to submit.
		 * @param	syncMask		Optional synchronization mask that determines if the submitted command buffer
		 *							depends on any other command buffers submitted on other queues.
		 *
		 *							This mask is only relevant if your command buffers are executing on different
		 *							queues, and are dependent. If they are executing on the same queue then they will
		 *							execute sequentially in the order they are submitted. Otherwise, if there is a
		 *							dependency you must make state it explicitly here.
		 */
		void SubmitCommandBuffer(const SPtr<render::GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask = GpuQueueMask::kAll);

		/**
		 * Presents the back-buffer image from the provided window onto the window, using the appropriate queue that supports present operations.
		 *
		 * @param	renderWindow		Window whose back-buffer to present.
		 * @param	syncMask			Optional synchronization mask that determines if the present operation
		 *								depends on any command buffers submitted on other queues.
		 */
		virtual void PresentRenderWindow(const SPtr<render::RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) = 0;

		/** Blocks the calling thread until all operations on the queue finish executing on the GPU. */
		virtual void WaitUntilIdle() = 0;

	protected:
		GpuQueue(GpuDevice& gpuDevice, GpuQueueType type, u32 index);

		/** Provides the same functionality as SubmitCommandBuffer(const SPtr<render::GpuCommandBuffer>&, GpuQueueMask), but makes the command buffer flush optional. */
		virtual void SubmitCommandBuffer(const SPtr<render::GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, bool flushTransferCommandBuffer) = 0;

		GpuDevice& mGpuDevice;
		GpuQueueType mType;
		u32 mIndex;
	};

	/**
	 * Provides access to a particular GPU device.
	 *
	 * @note	Thread safe.
	 */
	class B3D_EXPORT GpuDevice
	{
	public:
		virtual ~GpuDevice() = default;

		/** Initializes the GpuDevice. Should be called after construction but before any other operations. */
		virtual bool Initialize() = 0;

		/** Returns true if Initialize() has been called. */
		virtual bool IsInitialized() const = 0;

		virtual const GpuDeviceCapabilities& GetCapabilities() const = 0;

		/** Returns information about available output devices and their video modes. */
		virtual const VideoModeInfo& GetVideoModeInfo() const = 0;

		/** Query if a GPU program language is supported (for example "hlsl", "glsl"). Thread safe. */
		virtual bool IsGpuProgramLanguageSupported(const StringView& language) const = 0;

		/** Returns the number of queues supported for the specific usage. */
		virtual u32 GetQueueCount(GpuQueueType type) const = 0;

		/** Retrieves a queue with the specified usage and index. */
		virtual SPtr<GpuQueue> GetQueue(GpuQueueType type, u32 index) const = 0;

		/**
		 * Submits the command buffer for execution on an automatically retrieved queue.
		 *
		 * @param	commandBuffer	Command buffer to submit. Usage of the command buffer determines the queue to execute on.
		 * @param	syncMask		Optional synchronization mask that determines if the submitted command buffer
		 *							depends on any other command buffers submitted on other queues.
		 *
		 *							This mask is only relevant if your command buffers are executing on different
		 *							queues, and are dependent. If they are executing on the same queue then they will
		 *							execute sequentially in the order they are submitted. Otherwise, if there is a
		 *							dependency you must make state it explicitly here.
		 * @param	queueIndex		In case there are multiple queues supported with the command buffer's usage,
		 *							this determines which one to execute on.
		 */
		virtual void SubmitCommandBuffer(const SPtr<render::GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask = GpuQueueMask::kAll, u32 queueIndex = 0);

		/**
		 * Presents the back-buffer image from the provided window onto the window, using the appropriate queue that supports present operations.
		 *
		 * @param	renderWindow		Window whose back-buffer to present.
		 * @param	syncMask			Optional synchronization mask that determines if the present operation
		 *								depends on command buffers submitted on other queues.
		 */
		virtual void PresentRenderWindow(const SPtr<render::RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) = 0;

		/** Blocks the calling thread until all operations on the device finish. */
		virtual void WaitUntilIdle() = 0;

		virtual void BeginFrame() = 0;

		/** Notifies the device the rendering for the current frame has ended. See BeginFrame(). */
		virtual void EndFrame() = 0;

		/**
		 * Retrieves or creates a transfer command buffer for the current thread.
		 * The returned command buffer will be automatically submitted during EndFrame() or when
		 * SubmitTransferCommandBuffer() is called.
		 */
		const SPtr<render::GpuCommandBuffer>& GetOrCreateTransferCommandBuffer();

		/**
		 * Submits the transfer command buffer for the current thread. Optionally waits until the GPU is done processing it.
		 *
		 * @param	wait	If true, blocks until the GPU has finished executing the transfer command buffer.
		 */
		virtual void SubmitTransferCommandBuffers(bool wait = false);

		/************************************************************************/
		/* 								CREATION METHODS                   		*/
		/************************************************************************/

		/**
		 * Compiles the GPU program to an intermediate bytecode format. The bytecode can be cached and used for
		 * quicker compilation/creation of GPU programs.
		 */
		virtual SPtr<GpuProgramBytecode> CompileGpuProgramBytecode(const GpuProgramCreateInformation& createInformation) const = 0;

		/** Creates a command buffer pool that may be used for allocating command buffers. */
		virtual SPtr<render::GpuCommandBufferPool> CreateGpuCommandBufferPool(const render::GpuCommandBufferPoolCreateInformation& createInformation) = 0;

		/**
		 * Creates a new GPU texture.
		 *
		 * @param	createInformation		Object describing the texture to create.
		 * @param	deferredInitialize		If true, Initialize() will not be called on the returned object, and the caller is expected to call it himself, before first using the object.
		 */
		virtual SPtr<render::Texture> CreateTexture(const TextureCreateInformation& createInformation, bool deferredInitialize = false) = 0;

		/**
		 * Creates a new GPU buffer.
		 *
		 * @param	createInformation		Object describing the buffer to create.
		 * @param	deferredInitialize		If true, Initialize() will not be called on the returned object, and the caller is expected to call it himself, before first using the object.
		 */
		virtual SPtr<render::GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, bool deferredInitialize = false) = 0;

		/**
		 * Creates a new sampler state, or returns an existing one if one with the same create information was already created.
		 *
		 * @param	createInformation		Object describing the sampler state to create.
		 */
		virtual SPtr<SamplerState> FindOrCreateSamplerState(const SamplerStateCreateInformation& createInformation);

		/**
		 *  Creates a sampler state.
		 *
		 * @param	createInformation		Object describing the sampler state to create.
		 * @param	deferredInitialize		If true, Initialize() will not be called on the returned object, and the caller is expected to call it himself, before first using the object.
		 */
		virtual SPtr<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& createInformation, bool deferredInitialize = false) = 0;

		/**
		 * Creates a new query pool.
		 *
		 * @param	createInformation		Object describing the query pool to create.
		 */
		virtual SPtr<render::GpuQueryPool> CreateQueryPool(const render::GpuQueryPoolCreateInformation& createInformation) = 0;

		/** Create a new event query. */
		virtual SPtr<render::EventQuery> CreateEventQuery() = 0;

		/**
		 * Creates a new GPU program using the provided source code. If compilation fails or program is not supported
		 * GpuProgram::IsCompiled() will return false, and you will be able to retrieve the error message via GpuProgram::GetCompileErrorMessage().
		 *
		 * @param	createInformation		Object describing the program to create.
		 * @param	deferredInitialize		If true, Initialize() will not be called on the returned object, and the caller is expected to call it himself, before first using the object.
		 */
		virtual SPtr<GpuProgram> CreateGpuProgram(const GpuProgramCreateInformation& createInformation, bool deferredInitialize = false) = 0;

		/**
		 * Creates a set of GPU parameters that allow you to bind values to be used as input to GPU programs within a GPU pipeline with the provided layout.
		 *
		 * @param	parameterSetLayout		Layout that describes the GPU parameters for a particular set.
		 * @param	setIndex				Index of the parameter set that the object will be used for binding parameters for.
		 * @param	deferredInitialize		If true, Initialize() will not be called on the returned object, and the caller is expected to call it himself, before first using the object.
		 */
		virtual SPtr<render::GpuParameterSet> CreateGpuParameterSet(const SPtr<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex = 0, bool deferredInitialize = false) = 0;

		/**
		 * Creates a graphics pipeline.
		 *
		 * @param	createInformation		Object describing the pipeline to create.
		 * @param	deferredInitialize		If true, Initialize() will not be called on the returned object, and the caller is expected to call it himself, before first using the object.
		 */
		virtual SPtr<GpuGraphicsPipelineState> CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, bool deferredInitialize = false) = 0;

		/**
		 * Creates a compute pipeline.
		 *
		 * @param	createInformation		Object describing the pipeline to create.
		 * @param	deferredInitialize		If true, Initialize() will not be called on the returned object, and the caller is expected to call it himself, before first using the object.
		 */
		virtual SPtr<GpuComputePipelineState> CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, bool deferredInitialize = false) = 0;

		/**
		 * Creates a pipeline layout from a set of GPU program parameter descriptions.
		 *
		 * @param	createInformation		Object describing the layout to create.
		 */
		virtual SPtr<GpuPipelineParameterLayout> CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation) = 0;

		/**
		 * Creates a single GPU pipeline parameter set layout from a parameter description.
		 *
		 * @param	parameterDescription	Description of parameters in the set.
		 * @return							The created set layout.
		 */
		virtual SPtr<GpuPipelineParameterSetLayout> CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription) = 0;

		/**
		 * Creates a parameter set pool for allocating GPU parameter sets.
		 *
		 * @param	createInformation	Pool configuration including mode and capacity limits.
		 * @return						Created parameter set pool.
		 */
		virtual UPtr<GpuParameterSetPool> CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation) = 0;

		/************************************************************************/
		/* 								UTILITY METHODS                    		*/
		/************************************************************************/

		/** Contains a default matrix into a matrix suitable for use by this specific render system. */
		virtual void ConvertProjectionMatrix(const Matrix4& input, Matrix4& output) = 0;

		/**
		 * Generates a uniform buffer description and calculates per-uniform offsets for the provided buffer members.
		 * The generated offsets are GPU backend specific.
		 *
		 * @param	name			Name to assign the uniform block.
		 * @param	inOutUniforms	List of members in the uniform buffer. Only name, type and array size fields need to be
		 * 							populated, the rest will be populated when the method returns. If a parameter is a struct
		 * 							then the elementSize field needs to be populated with the size of the struct in bytes.
		 * @return					Descriptor for the uniform buffer holding the provided parameters as laid out by the
		 *							active GPU backend's layout.
		 */
		virtual GpuUniformBufferInformation GenerateUniformBufferInformation(const String& name, TArray<GpuUniformBufferMemberInformation>& inOutUniforms) = 0;

		/**
		 * Converts a GPU timestamp into a time in milliseconds.

		 * @param timestamp		Timestamp as the one retrieved from timestamp GPU query.
		 * @return				Time in milliseconds.
		 */
		virtual float ConvertTimestampToMilliseconds(u64 timestamp) = 0;
	protected:
		UPtr<GpuTransferBufferHelper> mTransferBufferHelper;

		mutable UnorderedMap<SamplerStateCreateInformation, SPtr<SamplerState>> mCachedSamplerStates;
		mutable Mutex mSamplerStateMutex;
	};

	/** @} */

} // namespace b3d
