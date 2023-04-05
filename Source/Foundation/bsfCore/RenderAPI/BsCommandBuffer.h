//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsColor.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup RenderAPI
		 *  @{
		 */

		/** Mask that determines synchronization between command buffers executing on different hardware queues. */
		class B3D_CORE_EXPORT CommandSyncMask
		{
		public:
			/**
			 * Registers a dependency on a command buffer. Use getMask() to get the new mask value after registering all
			 * dependencies.
			 */
			void AddDependency(const SPtr<CommandBuffer>& buffer);

			/** Returns a combined mask that contains all the required dependencies. */
			u32 GetMask() const { return mMask; }

			/** Uses the queue type and index to generate a mask with a bit set for that queue's global index. */
			static u32 GetGlobalQueueMask(GpuQueueType type, u32 queueIdx);

			/** Uses the queue type and index to generate a global queue index. */
			static u32 GetGlobalQueueIdx(GpuQueueType type, u32 queueIdx);

			/** Uses the global queue index to retrieve local queue index and queue type. */
			static u32 GetQueueIdxAndType(u32 globalQueueIdx, GpuQueueType& type);

		private:
			u32 mMask = 0;
		};

		/** Possible states that a CommandBuffer can be in. */
		enum class CommandBufferState
		{
			/** Command buffer doesn't have any commands recorded, nor has it been queued for execution. */
			Empty,

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

		/**
		 * Contains a list of render API commands that can be queued for execution on the GPU. User is allowed to populate the
		 * command buffer from any thread, ensuring render API command generation can be multi-threaded. Command buffers
		 * must always be created on the core thread. Same command buffer cannot be used on multiple threads simulateously
		 * without external synchronization.
		 */
		class B3D_CORE_EXPORT CommandBuffer
		{
		public:
			virtual ~CommandBuffer();

			/**
			 * Creates a new CommandBuffer.
			 *
			 * @param[in]	type		Determines what type of commands can be added to the command buffer.
			 * @param[in]	deviceIdx	Index of the GPU the command buffer will be used to queue commands on. 0 is always
			 *							the primary available GPU.
			 * @param[in]	queueIdx	Index of the GPU queue the command buffer will be used on. Command buffers with
			 *							the same index will execute sequentially, but command buffers with different queue
			 *							indices may execute in parallel, for a potential performance improvement.
			 *
			 *							Caller must ensure to synchronize operations executing on different queues via
			 *							sync masks. Command buffer dependant on another command buffer should provide a sync
			 *							mask when being submitted (see RenderAPI::executeCommands).
			 *
			 *							Queue indices are unique per buffer type (e.g. upload index 0 and graphics index 0 may
			 *							map to different queues internally). Must be in range [0, 7].
			 * @param[in]	secondary	If true the command buffer will not be allowed to execute on its own, but it can
			 *							be appended to a primary command buffer.
			 * @return					New CommandBuffer instance.
			 */
			static SPtr<CommandBuffer> Create(GpuQueueType type, u32 deviceIdx = 0, u32 queueIdx = 0, bool secondary = false);

			/** Assigns an name to the command buffer, primarily used for easier debugging. */
			virtual void SetName(const StringView& name) { mName = name; }

			/** Returns the type of queue the command buffer will execute on. */
			GpuQueueType GetType() const { return mType; }

			/** Returns the index of the queue the command buffer will execute on. */
			u32 GetQueueIdx() const { return mQueueIdx; }

			/** Returns the device index this buffer will execute on. */
			u32 GetDeviceIdx() const { return mDeviceIdx; }

			/** Returns the current state of the command buffer. */
			virtual CommandBufferState GetState() const = 0;

			/**
			 * Binds the parameters so that the following draw or dispatch call uses the provided parameters
			 * in their GPU programs. The caller must ensure the provided parameters match the bound graphics/compute pipeline
			 * at the time of the draw/dispatch call.
			 */
			virtual void SetGpuParameters(const SPtr<GpuParameters>& parameters);

			/** Sets a pipeline state that controls how will subsequent draw commands render primitives. */
			virtual void SetGpuGraphicsPipelineState(const SPtr<GpuGraphicsPipelineState>& pipelineState);

			/** Sets a pipeline state that controls how will subsequent dispatch commands execute. */
			virtual void SetGpuComputePipelineState(const SPtr<GpuComputePipelineState>& pipelineState);

			/**
			 * Sets the provided vertex buffers starting at the specified source index.	Set buffer to nullptr to clear the
			 * buffer at the specified index.
			 *
			 * @param	index			Index at which to start binding the vertex buffers.
			 * @param	buffers			A list of buffers to bind to the pipeline.
			 * @param	bufferCount		Number of buffers in the @p buffers list.
			 */
			virtual void SetVertexBuffers(u32 index, SPtr<GpuBuffer>* buffers, u32 bufferCount);

			/**
			 * Sets an index buffer to use when drawing. Indices in an index buffer reference vertices in the vertex buffer,
			 * which increases cache coherency and reduces the size of vertex buffers by eliminating duplicate data.
			 *
			 * @param	buffer			Index buffer to bind, null to unbind.
			 */
			virtual void SetIndexBuffer(const SPtr<GpuBuffer>& buffer);

			/**
			 * Sets the description of vertex elements in the vertex buffers that will be bound when executing the vertex GPU program.
			 *
			 * @param	vertexDescription	Vertex description to bind.
			 */
			virtual void SetVertexDescription(const SPtr<VertexDescription>& vertexDescription);

			/**
			 * Sets the draw operation that determines how to interpret the elements of the index or vertex buffers.
			 *
			 * @param	operation			Draw operation to enable.
			 */
			virtual void SetDrawOperation(DrawOperationType operation);

			/**
			 * Draw an object based on currently bound GPU programs, vertex declaration and vertex buffers. Draws directly from
			 * the vertex buffer without using indices.
			 *
			 * @param	vertexOffset	Offset into the currently bound vertex buffer to start drawing from.
			 * @param	vertexCount		Number of vertices to draw.
			 * @param	instanceCount	Number of times to draw the provided geometry, each time with an (optionally) separate per-instance data.
			 * @param	firstInstance	ID of the first instance to draw.
			 */
			virtual void Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, u32 firstInstance = 0);

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
			virtual void DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, u32 firstInstance = 0);

			/**
			 * Executes the currently bound compute shader.
			 *
			 * @param	groupCountX		Number of groups to start in the X direction. Must be in range [1, 65535].
			 * @param	groupCountY		Number of groups to start in the Y direction. Must be in range [1, 65535].
			 * @param	groupCountZ		Number of groups to start in the Z direction. Must be in range [1, 64].
			 */
			virtual void DispatchCompute(u32 groupCountX, u32 groupCountY = 1, u32 groupCountZ = 1);

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
			virtual void SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags = 0, RenderSurfaceMask loadMask = RT_NONE);

			/**
			 * Sets the active viewport that will be used for all following render operations.
			 *
			 * @param	area			Area of the viewport, in normalized ([0,1] range) coordinates.
			 */
			virtual void SetViewport(const Rect2& area);

			/**
			 * Clears the currently active render target.
			 *
			 * @param	buffers			Combination of one or more elements of FrameBufferType denoting which buffers are to be cleared.
			 * @param	color			The color to clear the color buffer with, if enabled.
			 * @param	depth			The value to initialize the depth buffer with, if enabled.
			 * @param	stencil			The value to initialize the stencil buffer with, if enabled.
			 * @param	targetMask		In case multiple render targets are bound, this allows you to control which ones to clear (0x01 first, 0x02 second, 0x04 third, etc., and combinations).
			 */
			virtual void ClearRenderTarget(u32 buffers, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF);

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
			virtual void ClearViewport(u32 buffers, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF);

			/**
			 * Allows you to set up a region in which rendering can take place. Coordinates are in pixels. No rendering will be
			 * done to render target pixels outside of the provided region.
			 *
			 * @param	left			Left border of the scissor rectangle, in pixels.
			 * @param	top				Top border of the scissor rectangle, in pixels.
			 * @param	right			Right border of the scissor rectangle, in pixels.
			 * @param	bottom			Bottom border of the scissor rectangle, in pixels.
			 */
			virtual void EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom);

			/** Disables scissor test set via EnableScissorTest(). */
			virtual void DisableScissorTest();

			/**
			 * Sets a reference value that will be used for stencil compare operations.
			 *
			 * @param	value			Reference value to set.
			 */
			virtual void SetStencilReferenceValue(u32 value);

			/**
			 * Surrounds all following commands with the provided label, until EndLabel() is called. This may be used by external
			 * tools for easier debugging.
			 */
			virtual void BeginLabel(const StringView& name);

			/** Closes the label scope as provided by the previous call to BeginLabel(). */
			virtual void EndLabel();

			/** Inserts a label at the specified location in the command buffer. This may be used by external tools for easier debugging. */
			virtual void InsertLabel(const StringView& name);

			/** Returns the shared pointer to the current object. */
			SPtr<CommandBuffer> GetShared() const { return mSelf.lock(); }

			/** Triggers when the command buffer finishes execution on the GPU. */
			Event<void()> OnDidComplete;

			/** Triggered just before a command buffer is about to be destroyed. Provided parameters determines if the command buffer was ever submitted or not. */
			Event<void(bool)> OnDestroyed;

		protected:
			CommandBuffer(GpuQueueType type, u32 deviceIdx, u32 queueIdx, bool secondary);

			/** Sets a pointer to itself. */
			void SetShared(const SPtr<CommandBuffer>& value) { mSelf = value; }

			GpuQueueType mType;
			String mName;
			u32 mDeviceIdx;
			u32 mQueueIdx;
			bool mIsSecondary;
			bool mIsSubmitted = false;

			WeakSPtr<CommandBuffer> mSelf;
		};

		/** @} */
	} // namespace ct
} // namespace bs
