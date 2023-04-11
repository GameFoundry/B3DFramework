//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "BsCorePrerequisites.h"
#include "BsGpuBackend.h"
#include "BsGpuDevice.h"
#include "RenderAPI/BsSamplerState.h"
#include "CoreThread/BsCommandQueue.h"
#include "RenderAPI/BsRenderTarget.h"
#include "RenderAPI/BsRenderTexture.h"
#include "RenderAPI/BsRenderWindow.h"
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsVertexDescription.h"
#include "RenderAPI/BsGpuDeviceCapabilities.h"
#include "Math/BsPlane.h"
#include "Utility/BsModule.h"
#include "Utility/BsEvent.h"

namespace bs
{
	class RenderAPIManager;

	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Provides access to ct::RenderAPI from the simulation thread. All the commands get queued on the core thread queue
	 * for the calling thread.
	 *
	 * @see		ct::RenderAPI
	 *
	 * @note	Sim thread only.
	 */
	class B3D_CORE_EXPORT RenderAPI
	{
	public:
		/**
		 * @see ct::RenderAPI::SetGpuParams()
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void SetGpuParams(const SPtr<GpuParameters>& gpuParams);

		/**
		 * @see ct::RenderAPI::SetGraphicsPipeline()
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void SetGraphicsPipeline(const SPtr<GpuGraphicsPipelineState>& pipelineState);

		/**
		 * @see ct::RenderAPI::SetComputePipeline()
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void SetComputePipeline(const SPtr<GpuComputePipelineState>& pipelineState);

		/**
		 * @see ct::RenderAPI::SetVertexBuffers()
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void SetVertexBuffers(u32 index, const Vector<SPtr<GpuBuffer>>& buffers);

		/**
		 * @see ct::RenderAPI::SetIndexBuffer()
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void SetIndexBuffer(const SPtr<GpuBuffer>& buffer);

		/**
		 * @see ct::RenderAPI::SetVertexDescription()
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void SetVertexDescription(const SPtr<VertexDescription>& vertexDescription);

		/**
		 * @see ct::RenderAPI::SetViewport()
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void SetViewport(const Rect2& area);

		/**
		 * @see ct::RenderAPI::SetStencilRef()
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void SetStencilRef(u32 value);

		/**
		 * @see ct::RenderAPI::SetDrawOperation()
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void SetDrawOperation(DrawOperationType op);

		/**
		 * @see ct::RenderAPI::SetRenderTarget()
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags = 0, RenderSurfaceMask loadMask = RT_NONE);

		/**
		 * @see ct::RenderAPI::ClearRenderTarget()
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void ClearRenderTarget(u32 buffers, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF);

		/**
		 * @see ct::RenderAPI::ClearViewport()
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void ClearViewport(u32 buffers, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF);

		/**
		 * @see ct::RenderAPI::SwapBuffers()
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void SwapBuffers(const SPtr<RenderTarget>& target);

		/**
		 * @see ct::RenderAPI::Draw()
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0);

		/**
		 * @see ct::RenderAPI::DrawIndexed()
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0);

		/**
		 * @see ct::RenderAPI::DispatchCompute()
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void DispatchCompute(u32 numGroupsX, u32 numGroupsY = 1, u32 numGroupsZ = 1);

		/** @copydoc ct::RenderAPI::ConvertProjectionMatrix */
		static void ConvertProjectionMatrix(const Matrix4& matrix, Matrix4& dest);
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup RenderAPI-Internal
		 *  @{
		 */

		/**
		 * Provides low-level API access to rendering commands (internally wrapping DirectX/OpenGL/Vulkan or similar).
		 *
		 * Methods that accept a CommandBuffer parameter get queued in the provided command buffer, and don't get executed until
		 * executeCommands() method is called. User is allowed to populate command buffers from non-core threads, but they all
		 * must get executed from the core thread.
		 *
		 * If a command buffer is not provivided to such methods, they execute immediately. Without a command buffer the methods
		 * are only allowed to be called from the core thread.
		 *
		 * @note	Accessible on any thread for methods accepting a CommandBuffer. Otherwise core thread unless specifically
		 *			noted otherwise on per-method basis.
		 */
		class B3D_CORE_EXPORT RenderAPI : public Module<RenderAPI>
		{
		public:
			RenderAPI();
			virtual ~RenderAPI();

			/**
			 * Returns the name of the rendering system.
			 *
			 * @note	Thread safe.
			 */
			virtual const StringID& GetName() const = 0;

			/**
			 * Applies a set of parameters that control execution of all currently bound GPU programs. These are the uniforms
			 * like textures, samplers, or uniform buffers. Caller is expected to ensure the provided parameters actually
			 * match the currently bound programs.
			 */
			virtual void SetGpuParams(const SPtr<GpuParameters>& gpuParams, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/**
			 * Sets a pipeline state that controls how will subsequent draw commands render primitives.
			 *
			 * @param[in]	pipelineState		Pipeline state to bind, or null to unbind.
			 * @param[in]	commandBuffer		Optional command buffer to queue the operation on. If not provided operation
			 *									is executed immediately. Otherwise it is executed when executeCommands() is
			 *									called. Buffer must support graphics operations.
			 *
			 * @see		GraphicsPipelineState
			 */
			virtual void SetGraphicsPipeline(const SPtr<GpuGraphicsPipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/**
			 * Sets a pipeline state that controls how will subsequent dispatch commands execute.
			 *
			 * @param[in]	pipelineState		Pipeline state to bind, or null to unbind.
			 * @param[in]	commandBuffer		Optional command buffer to queue the operation on. If not provided operation
			 *									is executed immediately. Otherwise it is executed when executeCommands() is
			 *									called. Buffer must support graphics operations.
			 */
			virtual void SetComputePipeline(const SPtr<GpuComputePipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/**
			 * Sets the active viewport that will be used for all render operations.
			 *
			 * @param[in]	area			Area of the viewport, in normalized ([0,1] range) coordinates.
			 * @param[in]	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
			 *								is executed immediately. Otherwise it is executed when executeCommands() is called.
			 *								Buffer must support graphics operations.
			 */
			virtual void SetViewport(const Rect2& area, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/**
			 * Allows you to set up a region in which rendering can take place. Coordinates are in pixels. No rendering will be
			 * done to render target pixels outside of the provided region.
			 *
			 * @param[in]	left			Left border of the scissor rectangle, in pixels.
			 * @param[in]	top				Top border of the scissor rectangle, in pixels.
			 * @param[in]	right			Right border of the scissor rectangle, in pixels.
			 * @param[in]	bottom			Bottom border of the scissor rectangle, in pixels.
			 * @param[in]	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
			 *								is executed immediately. Otherwise it is executed when executeCommands() is called.
			 *								Buffer must support graphics operations.
			 */
			virtual void EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/** Disables scissor test set via EnableScissorTest(). */
			virtual void DisableScissorTest(const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/**
			 * Sets a reference value that will be used for stencil compare operations.
			 *
			 * @param[in]	value			Reference value to set.
			 * @param[in]	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
			 *								is executed immediately. Otherwise it is executed when executeCommands() is called.
			 *								Buffer must support graphics operations.
			 */
			virtual void SetStencilRef(u32 value, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/**
			 * Sets the provided vertex buffers starting at the specified source index.	Set buffer to nullptr to clear the
			 * buffer at the specified index.
			 *
			 * @param[in]	index			Index at which to start binding the vertex buffers.
			 * @param[in]	buffers			A list of buffers to bind to the pipeline.
			 * @param[in]	numBuffers		Number of buffers in the @p buffers list.
			 * @param[in]	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
			 *								is executed immediately. Otherwise it is executed when executeCommands() is called.
			 *								Buffer must support graphics operations.
			 */
			virtual void SetVertexBuffers(u32 index, SPtr<GpuBuffer>* buffers, u32 numBuffers, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/**
			 * Sets an index buffer to use when drawing. Indices in an index buffer reference vertices in the vertex buffer,
			 * which increases cache coherency and reduces the size of vertex buffers by eliminating duplicate data.
			 *
			 * @param[in]	buffer			Index buffer to bind, null to unbind.
			 * @param[in]	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
			 *								is executed immediately. Otherwise it is executed when executeCommands() is called.
			 *								Buffer must support graphics operations.
			 */
			virtual void SetIndexBuffer(const SPtr<GpuBuffer>& buffer, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/**
			 * Sets the description of vertex elements in the vertex buffers that will be bound when executing the vertex GPU program.
			 *
			 * @param[in]	vertexDescription	Vertex description to bind.
			 * @param[in]	commandBuffer		Optional command buffer to queue the operation on. If not provided operation
			 *									is executed immediately. Otherwise it is executed when executeCommands() is
			 *									called. Buffer must support graphics operations.
			 */
			virtual void SetVertexDescription(const SPtr<VertexDescription>& vertexDescription, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/**
			 * Sets the draw operation that determines how to interpret the elements of the index or vertex buffers.
			 *
			 * @param[in]	op				Draw operation to enable.
			 * @param[in]	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
			 *								is executed immediately. Otherwise it is executed when executeCommands() is called.
			 *								Buffer must support graphics operations.
			 */
			virtual void SetDrawOperation(DrawOperationType op, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/**
			 * Draw an object based on currently bound GPU programs, vertex declaration and vertex buffers. Draws directly from
			 * the vertex buffer without using indices.
			 *
			 * @param	vertexOffset	Offset into the currently bound vertex buffer to start drawing from.
			 * @param	vertexCount		Number of vertices to draw.
			 * @param	instanceCount	Number of times to draw the provided geometry, each time with an (optionally)
			 *								separate per-instance data.
			 * @param	firstInstance	ID of the first instance to draw.
			 * @param	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
			 *							is executed immediately. Otherwise it is executed when executeCommands() is called.
			 *							Buffer must support graphics operations.
			 */
			virtual void Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, u32 firstInstance = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/**
			 * Draw an object based on currently bound GPU programs, vertex declaration, vertex and index buffers.
			 *
			 * @param	startIndex		Offset into the currently bound index buffer to start drawing from.
			 * @param	indexCount		Number of indices to draw.
			 * @param	vertexOffset	Offset to apply to each vertex index.
			 * @param	vertexCount		Number of vertices to draw.
			 * @param	instanceCount	Number of times to draw the provided geometry, each time with an (optionally) separate per-instance data.
			 * @param	firstInstance	ID of the first instance to draw.
			 * @param	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
			 *								is executed immediately. Otherwise it is executed when executeCommands() is called.
			 *								Buffer must support graphics operations.
			 */
			virtual void DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, u32 firstInstance = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/**
			 * Executes the currently bound compute shader.
			 *
			 * @param[in]	numGroupsX		Number of groups to start in the X direction. Must be in range [1, 65535].
			 * @param[in]	numGroupsY		Number of groups to start in the Y direction. Must be in range [1, 65535].
			 * @param[in]	numGroupsZ		Number of groups to start in the Z direction. Must be in range [1, 64].
			 * @param[in]	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
			 *								is executed immediately. Otherwise it is executed when executeCommands() is called.
			 *								Buffer must support compute or graphics operations.
			 */
			virtual void DispatchCompute(u32 numGroupsX, u32 numGroupsY = 1, u32 numGroupsZ = 1, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/**
			 * Swap the front and back buffer of the specified render target.
			 *
			 * @param[in]	target		Render target to perform the buffer swap on.
			 * @param[in]	syncMask	Optional synchronization mask that determines for which queues should the system wait
			 *							before performing the swap buffer operation. By default the system waits for all queues.
			 *							However if certain queues are performing non-rendering operations, or operations not
			 *							related to the provided render target, you can exclude them from the sync mask for
			 *							potentially better performance. You can use CommandSyncMask to generate a valid sync mask.
			 */
			virtual void SwapBuffers(const SPtr<RenderTarget>& target, u32 syncMask = 0xFFFFFFFF) = 0;

			/**
			 * Change the render target into which we want to draw.
			 *
			 * @param[in]	target					Render target to draw to.
			 * @param[in]	readOnlyFlags			Combination of one or more elements of FrameBufferType denoting which buffers
			 *										will be bound for read-only operations. This is useful for depth or stencil
			 *										buffers which need to be bound both for depth/stencil tests, as well as
			 *										shader reads.
			 * @param[in]	loadMask				Determines which render target surfaces will have their current contents
			 *										preserved. By default when a render target is bound its contents will be
			 *										lost. You might need to preserve contents if you need to perform blending
			 *										or similar operations with the existing contents of the render target.
			 *
			 *										Use the mask to select exactly which surfaces of the render target need
			 *										their contents preserved.
			 * @param[in]	commandBuffer			Optional command buffer to queue the operation on. If not provided operation
			 *										is executed immediately. Otherwise it is executed when executeCommands() is
			 *										called. Buffer must support graphics operations.
			 */
			virtual void SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags = 0, RenderSurfaceMask loadMask = RT_NONE, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/**
			 * Clears the currently active render target.
			 *
			 * @param[in]	buffers			Combination of one or more elements of FrameBufferType denoting which buffers are
			 *								to be cleared.
			 * @param[in]	color			The color to clear the color buffer with, if enabled.
			 * @param[in]	depth			The value to initialize the depth buffer with, if enabled.
			 * @param[in]	stencil			The value to initialize the stencil buffer with, if enabled.
			 * @param[in]	targetMask		In case multiple render targets are bound, this allows you to control which ones to
			 *									clear (0x01 first, 0x02 second, 0x04 third, etc., and combinations).
			 * @param[in]	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
			 *								is executed immediately. Otherwise it is executed when executeCommands() is called.
			 *								Buffer must support graphics operations.
			 */
			virtual void ClearRenderTarget(u32 buffers, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/**
			 * Clears the currently active viewport (meaning it clears just a sub-area of a render-target that is covered by the
			 * viewport, as opposed to clearRenderTarget() which always clears the entire render target).
			 *
			 * @param[in]	buffers			Combination of one or more elements of FrameBufferType denoting which buffers are to
			 *								be cleared.
			 * @param[in]	color			The color to clear the color buffer with, if enabled.
			 * @param[in]	depth			The value to initialize the depth buffer with, if enabled.
			 * @param[in]	stencil			The value to initialize the stencil buffer with, if enabled.
			 * @param[in]	targetMask		In case multiple render targets are bound, this allows you to control which ones to
			 *								clear (0x01 first, 0x02 second, 0x04 third, etc., and combinations).
			 * @param[in]	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
			 *								is executed immediately. Otherwise it is executed when executeCommands() is called.
			 *								Buffer must support graphics operations.
			 */
			virtual void ClearViewport(u32 buffers, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

			/** Appends all commands from the provided secondary command buffer into the primary command buffer. */
			virtual void AddCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary) = 0;

			/**
			 * Surrounds all following commands with the provided label, until EndLabel() is called. This may be used by external
			 * tools for easier debugging.
			 */
			virtual void BeginLabel(const StringView& name, const SPtr<CommandBuffer>& commandBuffer = nullptr) { }

			/** Closes the label scope as provided by the previous call to BeginLabel(). */
			virtual void EndLabel(const SPtr<CommandBuffer>& commandBuffer = nullptr) { }

			/**
			 * Inserts a label at the specified location in the command buffer. This may be used by external tools
			 * for easier debugging.
			 */
			virtual void InsertLabel(const StringView& name, const SPtr<CommandBuffer>& commandBuffer = nullptr) { }

			/**
			 * Prepares the backend for rendering a single frame. A frame involves a set of submitted command buffers followed by any optional present operations on the window surfaces.
			 * Must be followed by an EndFrame() call after all command buffers and present operations have been submitted.
			 */
			virtual void BeginFrame() {}

			/** Notifies the backend the rendering for the current frame has ended. See BeginFrame(). */
			virtual void EndFrame() {}

			/**
			 * Executes all commands in the provided command buffer. Command buffer cannot be secondary.
			 *
			 * @param	commandBuffer	Command buffer whose commands to execute. Set to null to submit the main command buffer.
			 * @param	queueIndex		Index of the queue to submit the command buffer on. This is the local index specific
			 *							to the type of the queue family the command buffer is allowed to be submitted on.
			 * @param	syncMask		Optional synchronization mask that determines if the submitted command buffer
			 *							depends on any other command buffers. Use the CommandSyncMask class to generate
			 *							a mask using existing command buffers.
			 *
			 *							This mask is only relevant if your command buffers are executing on different
			 *							hardware queues, and are somehow dependant. If they are executing on the same queue
			 *							(default) then they will execute sequentially in the order they are submitted.
			 *							Otherwise, if there is a dependency, you must make state it explicitly here.
			 *
			 * @note	Core thread only.
			 */
			virtual void SubmitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, u32 queueIndex = 0, u32 syncMask = 0xFFFFFFFF) = 0;

			/**	Waits until all the command buffers submitted thus far have finished executing on the GPU. */
			virtual void WaitUntilIdle() const = 0;

			/**
			 * Returns the currently active main command buffer instance. All commands queues without a user-provided command
			 * buffer will be queued on this command buffer. The command buffer instance will change after it has been submitted.
			 */
			virtual SPtr<CommandBuffer> GetMainCommandBuffer() const = 0;

			/** Returns the primary GPU on which to perform rendering. */
			virtual SPtr<GpuDevice> GetPrimaryGpuDevice() const = 0;

			/************************************************************************/
			/* 								UTILITY METHODS                    		*/
			/************************************************************************/

			/**
			 * Contains a default matrix into a matrix suitable for use by this specific render system.
			 *
			 * @note	Thread safe.
			 */
			virtual void ConvertProjectionMatrix(const Matrix4& matrix, Matrix4& dest) = 0;

			/**
			 * Generates a parameter block description and calculates per-parameter offsets for the provided gpu data
			 * parameters. The offsets are render API specific and correspond to std140 layout for OpenGL, and the default
			 * layout in DirectX.
			 *
			 * @param[in]	name	Name to assign the parameter block.
			 * @param[in]	params	List of parameters in the parameter block. Only name, type and array size fields need to be
			 * 						populated, the rest will be populated when the method returns. If a parameter is a struct
			 * 						then the elementSize field needs to be populated with the size of the struct in bytes.
			 * @return				Descriptor for the parameter block holding the provided parameters as laid out by the
			 *						default render API layout.
			 */
			virtual GpuDataParameterBlockInformation GenerateParamBlockDesc(const String& name, Vector<GpuDataParameterInformation>& params) = 0;

			/************************************************************************/
			/* 							INTERNAL METHODS				        	*/
			/************************************************************************/
		protected:
			/**
			 * Initializes the render API system and creates a primary render window.
			 *
			 * @note Sim thread only.
			 */
			SPtr<bs::RenderWindow> Initialize(const RENDER_WINDOW_DESC& primaryWindowDesc);

			/** Initializes the render API system. Called before the primary render window is created. */
			virtual void Initialize();

			/**
			 * Performs (optional) secondary initialization of the render API system. Called after the render window is
			 * created.
			 */
			virtual void InitializeWithWindow(const SPtr<RenderWindow>& primaryWindow);

			/**
			 * Shuts down the render API system and cleans up all resources.
			 *
			 * @note	Sim thread.
			 */
			void Destroy();

			/** Performs render API system shutdown on the core thread. */
			virtual void DestroyCore();

			/** Converts the number of vertices to number of primitives based on the specified draw operation. */
			u32 VertexCountToPrimCount(DrawOperationType type, u32 elementCount);

			/************************************************************************/
			/* 								INTERNAL DATA					       	*/
			/************************************************************************/
		protected:
			friend class bs::RenderAPIManager;

			SPtr<RenderTarget> mActiveRenderTarget;
			bool mActiveRenderTargetModified = false;
		};

		/** Shorthand for GpuDevice::GetCapabilities(). */
		inline const GpuDeviceCapabilities& GetGpuDeviceCapabilities(u32 deviceIndex = 0)
		{
			return GpuBackend::Instance().GetDevice(deviceIndex)->GetCapabilities();
		}

		/**	Provides easy access to render API. */
		inline RenderAPI& GetRenderAPI()
		{
			return RenderAPI::Instance();
		}

		/** @} */
	} // namespace ct
} // namespace bs
