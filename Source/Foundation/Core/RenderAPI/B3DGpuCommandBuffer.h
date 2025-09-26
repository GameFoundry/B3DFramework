//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DColor.h"
#include "Math/B3DArea2.h"
#include "Threading/B3DSingleConsumerQueue.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup RenderAPI
		 *  @{
		 */

		/** Creates a mask that determines synchronization between command buffers executing on different hardware queues. */
		class B3D_EXPORT CommandSyncMask
		{
		public:
			/** Uses the queue type and index to generate a mask with a bit set for that queue's global index. */
			static u32 GetGlobalQueueMask(GpuQueueUsage type, u32 queueIdx);

			/** Uses the queue type and index to generate a global queue index. */
			static u32 GetGlobalQueueIdx(GpuQueueUsage type, u32 queueIdx);

			/** Uses the global queue index to retrieve local queue index and queue type. */
			static u32 GetQueueIdxAndType(u32 globalQueueIdx, GpuQueueUsage& type);
		};

		/** Possible states that a CommandBuffer can be in. */
		enum class CommandBufferState
		{
			/** Command buffer doesn't have any commands recorded, nor has it been queued for execution. */
			Ready,

			/** Command buffer has one or multiple commands recorded, but they haven't been queued for execution. */
			Recording,

			/**
			 * Command buffer has been queued for execution, but still hasn't finished executing. Buffer that is
			 * executing cannot be modified or re-submitted for execution until done executing.
			 */
			Executing,

			/** Command buffer has been queued for execution and has finished executing. */
			Done
		};

		/** Object describing a GpuCommandBufferPool. */
		struct GpuCommandBufferPoolInformation
		{
			GpuQueueUsage Usage = GQT_GRAPHICS; /**< Determines which commands may be executed on the command buffer. Queue on which the command buffer is submitted must match this usage. */
			ThreadId Thread; /**< Thread on which the command buffer pool is allowed to be used on. Any created command buffers are also bound to this thread. */
		};

		/** Descriptor structure used for initialization of a GpuCommandBufferPool. */
		struct GpuCommandBufferPoolCreateInformation : GpuCommandBufferPoolInformation
		{
			GpuCommandBufferPoolCreateInformation() = default;
			GpuCommandBufferPoolCreateInformation(const GpuCommandBufferPoolInformation& other)
				:GpuCommandBufferPoolInformation(other)
			{ }

			/** Allocates a create information for a command buffer pool owned by the calling thread. */
			static GpuCommandBufferPoolCreateInformation CreateForThisThread(GpuQueueUsage usage = GQT_GRAPHICS)
			{
				GpuCommandBufferPoolCreateInformation createInformation;
				createInformation.Thread = B3D_CURRENT_THREAD_ID;
				createInformation.Usage = usage;

				return createInformation;
			}
		};

		/** Object describing a GpuCommandBuffer. */
		struct GpuCommandBufferInformation
		{
			String Name; /**< Name of the command buffer */
		};

		/** Descriptor structure used for initialization of a GpuCommandBufferPool. */
		struct GpuCommandBufferCreateInformation : GpuCommandBufferInformation 
		{
			GpuCommandBufferCreateInformation() = default;
			GpuCommandBufferCreateInformation(const GpuCommandBufferInformation& other)
				:GpuCommandBufferInformation(other)
			{ }

			/** Allocates a create information for a command buffer with the specified name. */
			static GpuCommandBufferCreateInformation Create(const StringView& name = "")
			{
				GpuCommandBufferCreateInformation createInformation;
				createInformation.Name = name;

				return createInformation;
			}
		};

		/**
		 * Allows creation of command buffers.
		 *
		 * All allocated command buffers may only be used on the GPU queues that have the subset of usage flags provided by this pool.
		 * Command buffer and all command buffers allocated from the command buffer may only be used on a single thread. Command buffers may only be used on another thread as part of command buffer submission.
		 */
		class B3D_EXPORT GpuCommandBufferPool
		{
		public:
			virtual ~GpuCommandBufferPool() = default;

			/** Returns queue that may be used for posting messages to the command buffer pool (e.g. command buffer completion notifies). */
			SingleConsumerQueue& GetMessageQueue() { return mMessageQueue; }

			/** Creates a new command buffer. */
			virtual SPtr<GpuCommandBuffer> Create(const GpuCommandBufferCreateInformation& createInformation) = 0;

			/** Attempts to find a free command buffer from the pool, or creates a new one if it cannot be found. */
			virtual SPtr<GpuCommandBuffer> FindOrCreate(const GpuCommandBufferCreateInformation& createInformation) = 0;

			/** Resets the command buffer pool, allowing all previously allocated command buffers to be re-used. Must be called only after all previously allocated command buffers have completed executing. */
			virtual void Reset() = 0;

			/** Destroys the pool. Will be called automatically on destruction, but may be called earlier if desired. */
			virtual void Destroy();

		protected:
			friend class b3d::GpuDevice;

			GpuCommandBufferPool(GpuDevice& gpuDevice, const GpuCommandBufferPoolCreateInformation& createInformation);

			/** Reports an error if the current thread is not the thread associated with the object. */
			void EnsureValidThread() const { B3D_DEBUG_ONLY(B3D_ENSURE(B3D_CURRENT_THREAD_ID == mInformation.Thread)); }

			GpuDevice& mGpuDevice;
			const GpuCommandBufferPoolInformation mInformation;
			SingleConsumerQueue mMessageQueue;
			bool mIsDestroyed = false;
		};

		/**
		 * Contains a list of render API commands that can be queued for execution on the GPU. User is allowed to populate the
		 * command buffer from any thread, ensuring render API command generation can be multi-threaded. Command buffers
		 * must always be created on the render thread. Same command buffer cannot be used on multiple threads simulateously
		 * without external synchronization.
		 */
		class B3D_EXPORT GpuCommandBuffer
		{
		public:
			virtual ~GpuCommandBuffer();

			/** Returns the GPU device the command buffer is created on. */
			GpuDevice& GetGpuDevice() const { return mGpuDevice; }

			/** Returns the usage that determines on which queue is the command buffer allowed to be submitted on, and which commands may be recorded. */
			GpuQueueUsage GetUsage() const { return mUsage; }

			/** Assigns an name to the command buffer, primarily used for easier debugging. */
			virtual void SetName(const StringView& name) { mName = name; }

			/** Returns the current state of the command buffer. */
			virtual CommandBufferState GetState() const = 0;

			/**
			 * Binds the parameters so that the following draw or dispatch call uses the provided parameters
			 * in their GPU programs. The caller must ensure the provided parameters match the bound graphics/compute pipeline
			 * at the time of the draw/dispatch call.
			 */
			virtual void SetGpuParameters(const SPtr<GpuParameters>& parameters) = 0;

			/**
			 * Applies an offset from which reads in a buffer should start in a GPU program. This allows caller to quickly change
			 * buffer contents as seen by the shader, without having to rebind GPU program parameters.
			 *
			 * @param bufferIndex		Dynamic buffer index, as retrieved from GpuPipelineParameterLayout of the currently bound GpuParameters.
			 * @param offset			Offset to apply. Must be within the range of the currently bound buffer size and respect hardware alignment requirements.
			 */
			virtual void SetDynamicBufferOffset(u32 bufferIndex, u32 offset) = 0;

			/** Sets a pipeline state that controls how will subsequent draw commands render primitives. */
			virtual void SetGpuGraphicsPipelineState(const SPtr<GpuGraphicsPipelineState>& pipelineState) = 0;

			/** Sets a pipeline state that controls how will subsequent dispatch commands execute. */
			virtual void SetGpuComputePipelineState(const SPtr<GpuComputePipelineState>& pipelineState) = 0;

			/**
			 * Sets the provided vertex buffers starting at the specified source index.	Set buffer to nullptr to clear the
			 * buffer at the specified index.
			 *
			 * @param	index			Index at which to start binding the vertex buffers.
			 * @param	buffers			A list of buffers to bind to the pipeline.
			 * @param	bufferCount		Number of buffers in the @p buffers list.
			 */
			virtual void SetVertexBuffers(u32 index, SPtr<GpuBuffer>* buffers, u32 bufferCount) = 0;

			/**
			 * Sets an index buffer to use when drawing. Indices in an index buffer reference vertices in the vertex buffer,
			 * which increases cache coherency and reduces the size of vertex buffers by eliminating duplicate data.
			 *
			 * @param	buffer			Index buffer to bind, null to unbind.
			 */
			virtual void SetIndexBuffer(const SPtr<GpuBuffer>& buffer) = 0;

			/**
			 * Sets the description of vertex elements in the vertex buffers that will be bound when executing the vertex GPU program.
			 *
			 * @param	vertexDescription	Vertex description to bind.
			 */
			virtual void SetVertexDescription(const SPtr<VertexDescription>& vertexDescription) = 0;

			/**
			 * Sets the draw operation that determines how to interpret the elements of the index or vertex buffers.
			 *
			 * @param	operation			Draw operation to enable.
			 */
			virtual void SetDrawOperation(DrawOperationType operation) = 0;

			/**
			 * Draw an object based on currently bound GPU programs, vertex declaration and vertex buffers. Draws directly from
			 * the vertex buffer without using indices.
			 *
			 * @param	vertexOffset	Offset into the currently bound vertex buffer to start drawing from.
			 * @param	vertexCount		Number of vertices to draw.
			 * @param	instanceCount	Number of times to draw the provided geometry, each time with an (optionally) separate per-instance data.
			 * @param	firstInstance	ID of the first instance to draw.
			 */
			virtual void Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, u32 firstInstance = 0) = 0;

			/**
			 * Draw an object based on currently bound GPU programs, vertex declaration, vertex and index buffers.
			 *
			 * @param	startIndex		Offset into the currently bound index buffer to start drawing from.
			 * @param	indexCount		Number of indices to draw.
			 * @param	vertexOffset	Offset to apply to each vertex index.
			 * @param	vertexCount		Number of vertices to draw.
			 * @param	instanceCount	Number of times to draw the provided geometry, each time with an (optionally) separate per-instance data.
			 * @param	firstInstance	ID of the first instance to draw.
			 */
			virtual void DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, u32 firstInstance = 0) = 0;

			/**
			 * Executes the currently bound compute shader.
			 *
			 * @param	groupCountX		Number of groups to start in the X direction. Must be in range [1, 65535].
			 * @param	groupCountY		Number of groups to start in the Y direction. Must be in range [1, 65535].
			 * @param	groupCountZ		Number of groups to start in the Z direction. Must be in range [1, 64].
			 */
			virtual void DispatchCompute(u32 groupCountX, u32 groupCountY = 1, u32 groupCountZ = 1) = 0;

			/**
			 * Change the render target into which to draw into.
			 *
			 * @param	target			Render target to draw to.
			 * @param	readOnlyFlags	Combination of one or more elements of FrameBufferType denoting which buffers
			 *							will be bound for read-only operations. This is useful for depth or stencil
			 *							buffers which need to be bound both for depth/stencil tests, as well as
			 *							shader reads.
			 * @param	loadMask		Determines which render target surfaces will have their current contents
			 *							preserved. By default when a render target is bound its contents will be
			 *							lost. You might need to preserve contents if you need to perform blending
			 *							or similar operations with the existing contents of the render target.
			 *
			 *							Use the mask to select exactly which surfaces of the render target need
			 *							their contents preserved.
			 */
			virtual void SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags = 0, RenderSurfaceMask loadMask = RT_NONE) = 0;

			/**
			 * Sets the active viewport that will be used for all following render operations.
			 *
			 * @param	area			Area of the viewport, in normalized ([0,1] range) coordinates.
			 */
			virtual void SetViewport(const Area2& area) = 0;

			/**
			 * Clears the currently active render target.
			 *
			 * @param	buffers			Combination of one or more elements of FrameBufferType denoting which buffers are to be cleared.
			 * @param	color			The color to clear the color buffer with, if enabled.
			 * @param	depth			The value to initialize the depth buffer with, if enabled.
			 * @param	stencil			The value to initialize the stencil buffer with, if enabled.
			 * @param	targetMask		In case multiple render targets are bound, this allows you to control which ones to clear (0x01 first, 0x02 second, 0x04 third, etc., and combinations).
			 */
			virtual void ClearRenderTarget(u32 buffers, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF) = 0;

			/**
			 * Clears the currently active viewport (meaning it clears just a sub-area of a render-target that is covered by the
			 * viewport, as opposed to ClearRenderTarget() which always clears the entire render target).
			 *
			 * @param	buffers			Combination of one or more elements of FrameBufferType denoting which buffers are to be cleared.
			 * @param	color			The color to clear the color buffer with, if enabled.
			 * @param	depth			The value to initialize the depth buffer with, if enabled.
			 * @param	stencil			The value to initialize the stencil buffer with, if enabled.
			 * @param	targetMask		In case multiple render targets are bound, this allows you to control which ones to clear (0x01 first, 0x02 second, 0x04 third, etc., and combinations).
			 */
			virtual void ClearViewport(u32 buffers, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF) = 0;

			/**
			 * Allows you to set up a region in which rendering can take place. Coordinates are in pixels. No rendering will be
			 * done to render target pixels outside of the provided region.
			 *
			 * @param	left			Left border of the scissor rectangle, in pixels.
			 * @param	top				Top border of the scissor rectangle, in pixels.
			 * @param	right			Right border of the scissor rectangle, in pixels.
			 * @param	bottom			Bottom border of the scissor rectangle, in pixels.
			 */
			virtual void EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom) = 0;

			/**
			 * Allows you to set up a region in which rendering can take place. Coordinates are in pixels. No rendering will be
			 * done to render target pixels outside of the provided region.
			 */
			virtual void EnableScissorTest(const Area2I& area) { EnableScissorTest(area.X, area.Y, area.X + area.Width, area.Y + area.Height); }

			/** Disables scissor test set via EnableScissorTest(). */
			virtual void DisableScissorTest() = 0;

			/**
			 * Sets a reference value that will be used for stencil compare operations.
			 *
			 * @param	value			Reference value to set.
			 */
			virtual void SetStencilReferenceValue(u32 value) = 0;

			/**
			 * Surrounds all following commands with the provided label, until EndLabel() is called. This may be used by external
			 * tools for easier debugging.
			 */
			virtual void BeginLabel(const StringView& name) = 0;

			/** Closes the label scope as provided by the previous call to BeginLabel(). */
			virtual void EndLabel() = 0;

			/** Inserts a label at the specified location in the command buffer. This may be used by external tools for easier debugging. */
			virtual void InsertLabel(const StringView& name) = 0;

			/** Ends command recording on the command buffer and makes it ready for submission. */
			virtual void End() = 0;

			/** Returns the shared pointer to the current object. */
			SPtr<GpuCommandBuffer> GetShared() const { return mSelf.lock(); }

			/** Triggers when the command buffer finishes execution on the GPU. */
			Event<void()> OnDidComplete;

			/** Triggered just before a command buffer is about to be destroyed. Provided parameters determines if the command buffer was ever submitted or not. */
			Event<void(bool)> OnDestroyed;

			/**
			 * @name Internal
			 * @{
			 */

			/** Sets a pointer to itself. */
			void SetShared(const SPtr<GpuCommandBuffer>& value) { mSelf = value; }

			/** @} */

		protected:
			friend class GpuCommandBufferPool;

			GpuCommandBuffer(GpuDevice& gpuDevice, ThreadId ownerThread, GpuQueueUsage queueType, const GpuCommandBufferCreateInformation& createInformation);

			/** Reports an error if the current thread is not the thread associated with the object. */
			void EnsureValidThread() const { B3D_DEBUG_ONLY(B3D_ENSURE(B3D_CURRENT_THREAD_ID == mOwnerThread)); }

			GpuDevice& mGpuDevice;
			const GpuCommandBufferCreateInformation mInformation;
			const GpuQueueUsage mUsage;
			const ThreadId mOwnerThread;
			String mName;
			bool mIsSubmitted = false;

			WeakSPtr<GpuCommandBuffer> mSelf;

		};

		/** @} */
	} // namespace render
} // namespace b3d
