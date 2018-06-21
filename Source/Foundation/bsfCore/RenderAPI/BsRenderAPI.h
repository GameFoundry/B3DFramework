//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "BsCorePrerequisites.h"
#include "RenderAPI/BsSamplerState.h"
#include "CoreThread/BsCommandQueue.h"
#include "RenderAPI/BsRenderAPICapabilities.h"
#include "RenderAPI/BsRenderTarget.h"
#include "RenderAPI/BsRenderTexture.h"
#include "RenderAPI/BsRenderWindow.h"
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsVertexDeclaration.h"
#include "Math/BsPlane.h"
#include "Utility/BsModule.h"
#include "Utility/BsEvent.h"

namespace bs
{
	class RenderAPIInfo;
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
	class BS_CORE_EXPORT RenderAPI
	{
	public:
		/** 
		 * @see ct::RenderAPI::setGpuParams() 
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void setGpuParams(const SPtr<GpuParams>& gpuParams);

		/** 
		 * @see ct::RenderAPI::setGraphicsPipeline() 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void setGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState);

		/** 
		 * @see ct::RenderAPI::setComputePipeline() 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void setComputePipeline(const SPtr<ComputePipelineState>& pipelineState);

		/** 
		 * @see ct::RenderAPI::setVertexBuffers() 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void setVertexBuffers(UINT32 index, const Vector<SPtr<VertexBuffer>>& buffers);

		/** 
		 * @see ct::RenderAPI::setIndexBuffer() 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void setIndexBuffer(const SPtr<IndexBuffer>& buffer);

		/** 
		 * @see ct::RenderAPI::setVertexDeclaration() 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void setVertexDeclaration(const SPtr<VertexDeclaration>& vertexDeclaration);

		/** 
		 * @see ct::RenderAPI::setViewport() 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void setViewport(const Rect2& area);

		/** 
		 * @see ct::RenderAPI::setStencilRef()
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void setStencilRef(UINT32 value);

		/** 
		 * @see ct::RenderAPI::setDrawOperation()  
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void setDrawOperation(DrawOperationType op);

		/** 
		 * @see ct::RenderAPI::setScissorRect() 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void setScissorRect(UINT32 left = 0, UINT32 top = 0, UINT32 right = 800, UINT32 bottom = 600);

		/** 
		 * @see ct::RenderAPI::setRenderTarget() 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void setRenderTarget(const SPtr<RenderTarget>& target, UINT32 readOnlyFlags = 0, 
			RenderSurfaceMask loadMask = RT_NONE);

		/** 
		 * @see ct::RenderAPI::clearRenderTarget() 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void clearRenderTarget(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, 
			UINT16 stencil = 0, UINT8 targetMask = 0xFF);

		/** 
		 * @see ct::RenderAPI::clearViewport() 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void clearViewport(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0,
			UINT8 targetMask = 0xFF);

		/** 
		 * @see ct::RenderAPI::swapBuffers() 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void swapBuffers(const SPtr<RenderTarget>& target);

		/** 
		 * @see ct::RenderAPI::draw() 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount = 0);

		/** 
		 * @see ct::RenderAPI::drawIndexed() 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount, 
			UINT32 instanceCount = 0);

		/** 
		 * @see ct::RenderAPI::dispatchCompute() 
		 * 
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		static void dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY = 1, UINT32 numGroupsZ = 1);

		/** @copydoc ct::RenderAPI::getVideoModeInfo */
		static const VideoModeInfo& getVideoModeInfo();

		/** @copydoc ct::RenderAPI::convertProjectionMatrix */
		static void convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest);

		/** @copydoc ct::RenderAPI::getAPIInfo */
		static const RenderAPIInfo& getAPIInfo();
	};

	/** Feature flags that describe which render API specific features are enabled. */
	enum class RenderAPIFeatureFlag
	{
		/** If set, vertex color order will be reversed before being sent to the shader. */
		VertexColorFlip			= 1 << 0,
		/** 
		 * If set, the Y axis in texture (UV) coordinates is assumed to be pointing up, instead of down (which is the 
		 * default). 
		 */
		UVYAxisUp				= 1 << 1,
		/**
		 * If set, the Y axis in normalized device coordinates (NDC) is assumed to be pointing down, instead of up (which
		 * is the default).
		 */
		NDCYAxisDown			= 1 << 2,
		/**
		 * If set, the matrices used by shaders are in column major order, instead of in row major (which is the default).
		 */
		ColumnMajorMatrices		= 1 << 3,
		/** 
		 * If set, the render API has native support for multi-threaded command buffer generation. Otherwise it is 
		 * emulated and using command buffers might not be beneficial. 
		 */
		MultiThreadedCB			= 1 << 4,
		/** If set, the render API supports unordered stores to a texture with more than one sample. */
		MSAAImageStores			= 1 << 5,
		/** 
		 * If set, the render API supports binds of parts of a texture (e.g. a single mip level, or a single element of a
		 * texture array). This only applies to texture reads and individual mip levels or layers can still be bound as
		 * render texture targets regardless of this flag.
		 */
		TextureViews			= 1 << 6,
		/** If set, the render API supports compute shaders. */
		Compute					= 1 << 7,
		/** If set, the render API supports load-store textures or buffers (AKA unordered access (UAV). */
		LoadStore				= 1 << 8,
		/** If set, GpuPrograms can cache shader bytecode for faster future compilation. */
		ByteCodeCaching			= 1 << 9,
		/**
		 * If set, the render API support rendering to multiple layers of a render texture at once (via a geometry shader).
		 */
		RenderTargetLayers		= 1 << 10,
	};

	typedef Flags<RenderAPIFeatureFlag> RenderAPIFeatures;
	BS_FLAGS_OPERATORS(RenderAPIFeatureFlag)

	/** Contains properties specific to a render API implementation. */
	class RenderAPIInfo
	{
	public:
		RenderAPIInfo(float horzTexelOffset, float vertTexelOffset, float minDepth, float maxDepth, 
			VertexElementType vertexColorType, RenderAPIFeatures featureFlags)
			: mHorizontalTexelOffset(horzTexelOffset), mVerticalTexelOffset(vertTexelOffset), mMinDepth(minDepth)
			, mMaxDepth(maxDepth), mVertexColorType(vertexColorType), mFeatureFlags(featureFlags)
		{ }

		/** Gets the native type used for vertex colors. */
		VertexElementType getColorVertexElementType() const { return mVertexColorType; }

		/** Gets horizontal texel offset used for mapping texels to pixels in this render system. */
		float getHorizontalTexelOffset() const { return mHorizontalTexelOffset; }

		/** Gets vertical texel offset used for mapping texels to pixels in this render system. */
		float getVerticalTexelOffset() const { return mVerticalTexelOffset; }

		/** Gets the minimum (closest) depth value used by this render system. */
		float getMinimumDepthInputValue() const { return mMinDepth; }

		/** Gets the maximum (farthest) depth value used by this render system. */
		float getMaximumDepthInputValue() const { return mMaxDepth; }

		/** Checks is a specific feature flag enabled. */
		bool isFlagSet(RenderAPIFeatureFlag flag) const { return mFeatureFlags.isSet(flag); }

	private:
		float mHorizontalTexelOffset = 0.0f;
		float mVerticalTexelOffset = 0.0f;
		float mMinDepth = 0.0f;
		float mMaxDepth = 1.0f;
		VertexElementType mVertexColorType = VET_COLOR_ABGR;
		RenderAPIFeatures mFeatureFlags;
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
	class BS_CORE_EXPORT RenderAPI : public Module<RenderAPI>
	{
	public:
		RenderAPI();
		virtual ~RenderAPI();

		/**
		 * Returns the name of the rendering system. 
		 *
		 * @note	Thread safe.
		 */
		virtual const StringID& getName() const = 0;

		/**
		 * Applies a set of parameters that control execution of all currently bound GPU programs. These are the uniforms
		 * like textures, samplers, or uniform buffers. Caller is expected to ensure the provided parameters actually
		 * match the currently bound programs.
		 */
		virtual void setGpuParams(const SPtr<GpuParams>& gpuParams, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

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
		virtual void setGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

		/**
		 * Sets a pipeline state that controls how will subsequent dispatch commands execute.
		 *
		 * @param[in]	pipelineState		Pipeline state to bind, or null to unbind.
		 * @param[in]	commandBuffer		Optional command buffer to queue the operation on. If not provided operation
		 *									is executed immediately. Otherwise it is executed when executeCommands() is 
		 *									called. Buffer must support graphics operations.
		 */
		virtual void setComputePipeline(const SPtr<ComputePipelineState>& pipelineState,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

		/**
		 * Sets the active viewport that will be used for all render operations.
		 *
		 * @param[in]	area			Area of the viewport, in normalized ([0,1] range) coordinates.
		 * @param[in]	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
		 *								is executed immediately. Otherwise it is executed when executeCommands() is called.
		 *								Buffer must support graphics operations.
		 */
		virtual void setViewport(const Rect2& area, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

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
		virtual void setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

		/**
		 * Sets a reference value that will be used for stencil compare operations.
		 *
		 * @param[in]	value			Reference value to set.
		 * @param[in]	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
		 *								is executed immediately. Otherwise it is executed when executeCommands() is called.
		 *								Buffer must support graphics operations.
		 */
		virtual void setStencilRef(UINT32 value, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

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
		virtual void setVertexBuffers(UINT32 index, SPtr<VertexBuffer>* buffers, UINT32 numBuffers, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

		/**
		 * Sets an index buffer to use when drawing. Indices in an index buffer reference vertices in the vertex buffer, 
		 * which increases cache coherency and reduces the size of vertex buffers by eliminating duplicate data.
		 *
		 * @param[in]	buffer			Index buffer to bind, null to unbind.
		 * @param[in]	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
		 *								is executed immediately. Otherwise it is executed when executeCommands() is called.
		 *								Buffer must support graphics operations.
		 */
		virtual void setIndexBuffer(const SPtr<IndexBuffer>& buffer, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

		/**
		 * Sets the vertex declaration to use when drawing. Vertex declaration is used to decode contents of a single 
		 * vertex in a vertex buffer.
		 *
		 * @param[in]	vertexDeclaration	Vertex declaration to bind.
		 * @param[in]	commandBuffer		Optional command buffer to queue the operation on. If not provided operation
		 *									is executed immediately. Otherwise it is executed when executeCommands() is 
		 *									called. Buffer must support graphics operations.
		 */
		virtual void setVertexDeclaration(const SPtr<VertexDeclaration>& vertexDeclaration, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

		/** 
		 * Sets the draw operation that determines how to interpret the elements of the index or vertex buffers. 
		 *
		 * @param[in]	op				Draw operation to enable.
		 * @param[in]	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
		 *								is executed immediately. Otherwise it is executed when executeCommands() is called.
		 *								Buffer must support graphics operations.
		 */
		virtual void setDrawOperation(DrawOperationType op, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

		/**
		 * Draw an object based on currently bound GPU programs, vertex declaration and vertex buffers. Draws directly from
		 * the vertex buffer without using indices.
		 *
		 * @param[in]	vertexOffset	Offset into the currently bound vertex buffer to start drawing from.
		 * @param[in]	vertexCount		Number of vertices to draw.
		 * @param[in]	instanceCount	Number of times to draw the provided geometry, each time with an (optionally)
		 *								separate per-instance data.
		 * @param[in]	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
		 *								is executed immediately. Otherwise it is executed when executeCommands() is called.
		 *								Buffer must support graphics operations.
		 */
		virtual void draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount = 0, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

		/** 
		 * Draw an object based on currently bound GPU programs, vertex declaration, vertex and index buffers. 
		 *
		 * @param[in]	startIndex		Offset into the currently bound index buffer to start drawing from.
		 * @param[in]	indexCount		Number of indices to draw.
		 * @param[in]	vertexOffset	Offset to apply to each vertex index.
		 * @param[in]	vertexCount		Number of vertices to draw.
		 * @param[in]	instanceCount	Number of times to draw the provided geometry, each time with an (optionally)
		 *								separate per-instance data.
		 * @param[in]	commandBuffer	Optional command buffer to queue the operation on. If not provided operation
		 *								is executed immediately. Otherwise it is executed when executeCommands() is called.
		 *								Buffer must support graphics operations.
		 */
		virtual void drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount, 
			UINT32 instanceCount = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

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
		virtual void dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY = 1, UINT32 numGroupsZ = 1, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

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
		virtual void swapBuffers(const SPtr<RenderTarget>& target, UINT32 syncMask = 0xFFFFFFFF) = 0;

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
		virtual void setRenderTarget(const SPtr<RenderTarget>& target, UINT32 readOnlyFlags = 0,
			RenderSurfaceMask loadMask = RT_NONE, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

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
		virtual void clearRenderTarget(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, 
			UINT16 stencil = 0, UINT8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

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
		virtual void clearViewport(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, 
			UINT16 stencil = 0, UINT8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) = 0;

		/** Appends all commands from the provided secondary command buffer into the primary command buffer. */
		virtual void addCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary) = 0;

		/** 
		 * Executes all commands in the provided command buffer. Command buffer cannot be secondary.
		 *
		 * @param[in]	commandBuffer	Command buffer whose commands to execute. Set to null to submit the main command
		 *								buffer.
		 * @param[in]	syncMask		Optional synchronization mask that determines if the submitted command buffer
		 *								depends on any other command buffers. Use the CommandSyncMask class to generate
		 *								a mask using existing command buffers.
		 *								
		 *								This mask is only relevant if your command buffers are executing on different
		 *								hardware queues, and are somehow dependant. If they are executing on the same queue
		 *								(default) then they will execute sequentially in the order they are submitted.
		 *								Otherwise, if there is a dependency, you must make state it explicitly here.
		 *
		 * @note	Core thread only.
		 */
		virtual void submitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, UINT32 syncMask = 0xFFFFFFFF) = 0;

		/**
		 * Gets the capabilities of a specific GPU.
		 * 
		 * @param[in]	deviceIdx	Index of the device to get the capabilities for.
		 *
		 * @note	Thread safe.
		 */
		const RenderAPICapabilities& getCapabilities(UINT32 deviceIdx) const;

		/** Returns the number of devices supported by this render API. */
		UINT32 getNumDevices() const { return mNumDevices; }

		/**
		 * Returns information about available output devices and their video modes.
		 *
		 * @note	Thread safe.
		 */
		const VideoModeInfo& getVideoModeInfo() const { return *mVideoModeInfo; }

		/************************************************************************/
		/* 								UTILITY METHODS                    		*/
		/************************************************************************/

		/**
		 * Contains a default matrix into a matrix suitable for use by this specific render system.
		 *
		 * @note	Thread safe.
		 */
		virtual void convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest) = 0;

		/** 
		 * Returns information about the specific API implementation. 
		 *
		 * @note	Thread safe.
		 */
		virtual const RenderAPIInfo& getAPIInfo() const = 0;

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
		virtual GpuParamBlockDesc generateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params) = 0;

		/************************************************************************/
		/* 							INTERNAL METHODS				        	*/
		/************************************************************************/
	protected:
		/**
		 * Initializes the render API system and creates a primary render window.
		 *
		 * @note Sim thread only.
		 */
		SPtr<bs::RenderWindow> initialize(const RENDER_WINDOW_DESC& primaryWindowDesc);

		/** Initializes the render API system. Called before the primary render window is created. */
		virtual void initialize();

		/**
		 * Performs (optional) secondary initialization of the render API system. Called after the render window is 
		 * created.
		 */
		virtual void initializeWithWindow(const SPtr<RenderWindow>& primaryWindow);

		/**
		 * Shuts down the render API system and cleans up all resources.
		 *
		 * @note	Sim thread.
		 */
		void destroy();

		/** Performs render API system shutdown on the core thread. */
		virtual void destroyCore();

		/** Converts the number of vertices to number of primitives based on the specified draw operation. */
		UINT32 vertexCountToPrimCount(DrawOperationType type, UINT32 elementCount);

		/************************************************************************/
		/* 								INTERNAL DATA					       	*/
		/************************************************************************/
	protected:
		friend class bs::RenderAPIManager;

		SPtr<RenderTarget> mActiveRenderTarget;

		RenderAPICapabilities* mCurrentCapabilities;
		UINT32 mNumDevices;
		SPtr<VideoModeInfo> mVideoModeInfo;
	};

	/** @} */
	}
}