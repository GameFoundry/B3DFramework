//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	namespace ct
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

	/**
	 * Specifies a queue on which command buffers can be submitted on.
	 *
	 * @note	Thread safe.
	 */
	class B3D_CORE_EXPORT GpuQueue
	{
	public:
		virtual ~GpuQueue() = default;

		/** Determines which type of command buffer commands can be used on the command buffers submitted on the queue. */
		GpuQueueUsage GetUsage() const { return mUsage; }

		/** Returns the unique index of the queue, for its type. */
		u32 GetIndex() const { return mIndex; }

		/**
		 * Submits the command buffer for execution on the specified queue.
		 *
		 * @param	commandBuffer	Command buffer to submit.
		 * @param	syncMask		Optional synchronization mask that determines if the submitted command buffer
		 *							depends on any other command buffers submitted on other queues. You may use the
		 *							CommandSyncMask class to generate a mask.
		 *
		 *							This mask is only relevant if your command buffers are executing on different
		 *							queues, and are dependent. If they are executing on the same queue then they will
		 *							execute sequentially in the order they are submitted. Otherwise, if there is a
		 *							dependency you must make state it explicitly here.
		 */
		virtual void SubmitCommandBuffer(const SPtr<ct::GpuCommandBuffer>& commandBuffer, u32 syncMask = 0) = 0;

		/** Submits multiple command buffers at the same time. See SubmitCommandBuffer(). */
		virtual void SubmitCommandBuffers(const ArrayView<SPtr<ct::GpuCommandBuffer>>& commandBuffer, u32 syncMask = 0) = 0;

		/**
		 * Returns a command buffer that is to be used for transfer operations when user doesn't provide an explicit command buffer.
		 * Transfer command buffers on all queues should be submitted before any regular explicit command buffer submission, or at the end of frame.
		 * Each thread calling this method will retrieve a separate command buffer.
		 */
		virtual SPtr<ct::GpuCommandBuffer> GetOrCreateTransferCommandBuffer();

		/** Submits the active transfer command buffers for all threads. All existing command buffers are invalidated. */
		virtual void SubmitTransferCommandBuffers();

	protected:
		/** Information about a transfer command buffer associated with a particular thread. */
		struct PerThreadTransferCommandBufferInformation
		{
			SPtr<ct::GpuCommandBufferPool> CommandBufferPool; /**< Pool for allocating the command buffers. */
			SPtr<ct::GpuCommandBuffer> CurrentTransferCommandBuffer; /**< Currently active transfer buffer, if any. */
		};

		GpuQueue(GpuDevice& gpuDevice, GpuQueueUsage usage, u32 index);

		GpuDevice& mGpuDevice;
		GpuQueueUsage mUsage;
		u32 mIndex;

		mutable Mutex mMutex;
		mutable UnorderedMap<ThreadId, PerThreadTransferCommandBufferInformation> mTransferCommandBuffers;
	};

	/**
	 * Provides access to a particular GPU device.
	 *
	 * @note	Thread safe.
	 */
	class B3D_CORE_EXPORT GpuDevice
	{
	public:
		virtual ~GpuDevice() = default;

		// TODO - Doc
		virtual bool IsInitialized() const = 0;
		virtual bool Initialize() = 0;

		virtual const GpuDeviceCapabilities& GetCapabilities() = 0;

		/** Returns information about available output devices and their video modes. */
		virtual const VideoModeInfo& GetVideoModeInfo() const = 0;

		/** Query if a GPU program language is supported (for example "hlsl", "glsl"). Thread safe. */
		virtual bool IsGpuProgramLanguageSupported(const StringView& language) const = 0;

		/** Returns the number of queues supported for the specific usage. */
		virtual u32 GetQueueCount(GpuQueueUsage usage) const = 0;

		/** Retrieves a queue with the specified usage and index. */
		virtual SPtr<GpuQueue> GetQueue(GpuQueueUsage usage, u32 index) const = 0;

		/** Submits all non-empty transfer command buffers on all queues and optionally waits until the GPU is done processing them. */
		virtual void SubmitTransferCommandBuffers(bool wait = false) = 0;

		/**
		 * Compiles the GPU program to an intermediate bytecode format. The bytecode can be cached and used for
		 * quicker compilation/creation of GPU programs.
		 */
		virtual SPtr<GpuProgramBytecode> CompileGpuProgramBytecode(const GpuProgramCreateInformation& createInformation) const = 0;

		/** Creates a command buffer pool that may be used for allocating command buffers. */
		virtual SPtr<ct::GpuCommandBufferPool> CreateGpuCommandBufferPool(const ct::GpuCommandBufferPoolCreateInformation& createInformation) = 0;

		/**
		 * Creates a new GPU buffer.
		 *
		 * @param	createInformation		Object describing the buffer to create.
		 * @param	deferredInitialize		If true, Initialize() will not be called on the returned object, and the caller is expected to call it himself, before first using the object.
		 */
		virtual SPtr<ct::GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, bool deferredInitialize = false) = 0;

		/** Create a new event query. */
		virtual SPtr<ct::EventQuery> CreateEventQuery() = 0;

		/** Creates a new timer query. */
		virtual SPtr<ct::TimerQuery> CreateTimerQuery() = 0;

		/**
		 * Creates a new occlusion query.
		 *
		 * @param isBinary		If query is binary it will not give you an exact count of samples rendered, but will
		 *						instead just return 0 (no samples were rendered) or 1 (one or more samples were
		 *						rendered). Binary queries can return sooner as they potentially do not need to wait
		 *						until all of the geometry is rendered.
		 */
		virtual SPtr<ct::OcclusionQuery> CreateOcclusionQuery(bool isBinary) = 0;

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
		 * @param	parameterLayout			Layout that describes the GPU parameters, as retrieved from the GPU pipeline.
		 * @param	deferredInitialize		If true, Initialize() will not be called on the returned object, and the caller is expected to call it himself, before first using the object.
		 */
		virtual SPtr<ct::GpuParameters> CreateGpuParameters(const SPtr<GpuPipelineParameterLayout>& parameterLayout, bool deferredInitialize = false) = 0;

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
		 * @param	deferredInitialize		If true, Initialize() will not be called on the returned object, and the caller is expected to call it himself, before first using the object.
		 */
		virtual SPtr<GpuPipelineParameterLayout> CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation, bool deferredInitialize = false) = 0;
	};

	/** @} */

} // namespace bs
