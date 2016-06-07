//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once
#include "BsCorePrerequisites.h"
#include "BsSamplerState.h"
#include "BsCommandQueue.h"
#include "BsDrawOps.h"
#include "BsRenderAPICapabilities.h"
#include "BsRenderTarget.h"
#include "BsRenderTexture.h"
#include "BsRenderWindow.h"
#include "BsGpuProgram.h"
#include "BsVertexDeclaration.h"
#include "BsPlane.h"
#include "BsModule.h"
#include "BsEvent.h"

namespace BansheeEngine
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	class RenderAPIInfo;

	/**
	 * Version of the render API interface usable from the sim thread. All the commands	get queued on the accessor provided
	 * to each method and will be executed on the core thread later.
	 *
	 * @see		RenderAPICore
	 *
	 * @note	Sim thread only.
	 */
	class BS_CORE_EXPORT RenderAPI
	{
	public:
		/** 
		 * @copydoc RenderAPICore::disableTextureUnit()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void disableTextureUnit(CoreAccessor& accessor, GpuProgramType gptype, UINT16 texUnit);

		/**  
		 * @copydoc RenderAPICore::setTexture()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void setTexture(CoreAccessor& accessor, GpuProgramType gptype, UINT16 texUnit, bool enabled, 
			const SPtr<Texture> &texPtr);

		/**  
		 * @copydoc RenderAPICore::setLoadStoreTexture()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void setLoadStoreTexture(CoreAccessor& accessor, GpuProgramType gptype, UINT16 texUnit, bool enabled, 
			const SPtr<Texture>& texPtr, const TextureSurface& surface);

		/**  
		 * @copydoc RenderAPICore::setBuffer()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void setBuffer(CoreAccessor& accessor, GpuProgramType gptype, UINT16 unit, const SPtr<GpuBuffer>& buffer,
			bool loadStore = false);

		/** 
		 * @copydoc RenderAPICore::setSamplerState()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void setSamplerState(CoreAccessor& accessor, GpuProgramType gptype, UINT16 texUnit, 
			const SPtr<SamplerState>& samplerState);

		/**  
		 * @copydoc RenderAPICore::setBlendState()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void setBlendState(CoreAccessor& accessor, const SPtr<BlendState>& blendState);

		/** 
		 * @copydoc RenderAPICore::setRasterizerState()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void setRasterizerState(CoreAccessor& accessor, const SPtr<RasterizerState>& rasterizerState);

		/** 
		 * @copydoc RenderAPICore::setDepthStencilState()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void setDepthStencilState(CoreAccessor& accessor, const SPtr<DepthStencilState>& depthStencilState, 
			UINT32 stencilRefValue);

		/** 
		 * Sets the provided vertex buffers starting at the specified source index.	Set buffer to nullptr to clear the
		 * buffer at the specified index.
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 * @param[in]	index		Index at which to start binding the vertex buffers.
		 * @param[in]	buffers		A list of buffers to bind to the pipeline.
		 */
		static void setVertexBuffers(CoreAccessor& accessor, UINT32 index, const Vector<SPtr<VertexBuffer>>& buffers);

		/** 
		 * @copydoc RenderAPICore::setIndexBuffer()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void setIndexBuffer(CoreAccessor& accessor, const SPtr<IndexBuffer>& buffer);

		/** 
		 * @copydoc RenderAPICore::setVertexDeclaration()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void setVertexDeclaration(CoreAccessor& accessor, const SPtr<VertexDeclaration>& vertexDeclaration);

		/** 
		 * @copydoc RenderAPICore::setViewport()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void setViewport(CoreAccessor& accessor, const Rect2& area);

		/** 
		 * @copydoc RenderAPICore::setDrawOperation() 
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void setDrawOperation(CoreAccessor& accessor, DrawOperationType op);

		/** 
		 * @copydoc RenderAPICore::setClipPlanes() 
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void setClipPlanes(CoreAccessor& accessor, const PlaneList& clipPlanes);

		/** 
		 * @copydoc RenderAPICore::addClipPlane(const Plane&) 
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void addClipPlane(CoreAccessor& accessor, const Plane& p);

		/** 
		 * @copydoc RenderAPICore::resetClipPlanes() 
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void resetClipPlanes(CoreAccessor& accessor);

		/** 
		 * @copydoc RenderAPICore::setScissorRect() 
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void setScissorRect(CoreAccessor& accessor, UINT32 left = 0, UINT32 top = 0, UINT32 right = 800, UINT32 bottom = 600);

		/** 
		 * @copydoc RenderAPICore::setRenderTarget()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void setRenderTarget(CoreAccessor& accessor, const SPtr<RenderTarget>& target, bool readOnlyDepthStencil = false);

		/** 
		 * @copydoc RenderAPICore::bindGpuProgram()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void bindGpuProgram(CoreAccessor& accessor, const SPtr<GpuProgram>& prg);

		/** 
		 * @copydoc RenderAPICore::unbindGpuProgram()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void unbindGpuProgram(CoreAccessor& accessor, GpuProgramType gptype);

		/** 
		 * @copydoc RenderAPICore::setConstantBuffers()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void setConstantBuffers(CoreAccessor& accessor, GpuProgramType gptype, const SPtr<GpuParams>& params);

		/** 
		 * @copydoc RenderAPICore::setGpuParams()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void setGpuParams(CoreAccessor& accessor, GpuProgramType gptype, const SPtr<GpuParams>& params);

		/** 
		 * @copydoc RenderAPICore::beginFrame()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void beginRender(CoreAccessor& accessor);

		/** 
		 * @copydoc RenderAPICore::endFrame()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void endRender(CoreAccessor& accessor);

		/** 
		 * @copydoc RenderAPICore::clearRenderTarget() 
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void clearRenderTarget(CoreAccessor& accessor, UINT32 buffers, 
			const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0, UINT8 targetMask = 0xFF);

		/** 
		 * @copydoc RenderAPICore::clearViewport()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void clearViewport(CoreAccessor& accessor, UINT32 buffers, const Color& color = Color::Black, 
			float depth = 1.0f, UINT16 stencil = 0, UINT8 targetMask = 0xFF);

		/** 
		 * @copydoc RenderAPICore::swapBuffers()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void swapBuffers(CoreAccessor& accessor, const SPtr<RenderTarget>& target);

		/** 
		 * @copydoc RenderAPICore::draw()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void draw(CoreAccessor& accessor, UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount = 0);

		/** 
		 * @copydoc RenderAPICore::drawIndexed()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void drawIndexed(CoreAccessor& accessor, UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, 
			UINT32 vertexCount, UINT32 instanceCount = 0);

		/** 
		 * @copydoc RenderAPICore::dispatchCompute()
		 *
		 * @param[in]	accessor	Accessor on which will this command be queued for execution.
		 */
		static void dispatchCompute(CoreAccessor& accessor, UINT32 numGroupsX, UINT32 numGroupsY = 1, UINT32 numGroupsZ = 1);

		/** @copydoc RenderAPICore::getVideoModeInfo */
		static const VideoModeInfo& getVideoModeInfo();

		/** @copydoc RenderAPICore::convertProjectionMatrix */
		static void convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest);

		/** @copydoc RenderAPICore::getAPIInfo */
		static const RenderAPIInfo& getAPIInfo();
	};

	/** Contains properties specific to a render API implementation. */
	class RenderAPIInfo
	{
	public:
		RenderAPIInfo(float horzTexelOffset, float vertTexelOffset, float minDepth, float maxDepth, 
			VertexElementType vertexColorType, bool vertexColorFlip, bool ndcVerticalTopToBottom, bool columnMajorMatrices)
			: mHorizontalTexelOffset(horzTexelOffset), mVerticalTexelOffset(vertTexelOffset), mMinDepth(minDepth)
			, mMaxDepth(maxDepth), mVertexColorType(vertexColorType), mVertexColorFlip(vertexColorFlip)
			, mNDCYAxisDown(ndcVerticalTopToBottom), mColumnMajorMatrices(columnMajorMatrices)
		{
			
		}

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

		/** Checks if vertex color needs to be flipped before sent to the shader. */
		bool getVertexColorFlipRequired() const { return mVertexColorFlip; }

		/** Checks whether GPU programs expect matrices in column major format. */
		bool getGpuProgramHasColumnMajorMatrices() const { return mColumnMajorMatrices; }
		
		/** 
		 * Returns true if Y axis in normalized device coordinates points down, false if up. If axis is pointing down the
		 * NDC at the top is -1, and at the bottom is 1, otherwise reverse.
		 */
		bool getNDCYAxisDown() const { return mNDCYAxisDown; }

	private:
		float mHorizontalTexelOffset = 0.0f;
		float mVerticalTexelOffset = 0.0f;
		float mMinDepth = 0.0f;
		float mMaxDepth = 1.0f;
		VertexElementType mVertexColorType = VET_COLOR_ABGR;
		bool mVertexColorFlip = false;
		bool mNDCYAxisDown = true;
		bool mColumnMajorMatrices = false;
	};

	/** @} */
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Render system provides base functionality for a rendering API like DirectX or OpenGL. Most of the class is abstract
	 * and specific subclass for each rendering API needs to be implemented.
	 *
	 * @note	Core thread only unless specifically noted otherwise on per-method basis.
	 */
	class BS_CORE_EXPORT RenderAPICore : public Module<RenderAPICore>
	{
	public:
		RenderAPICore();
		virtual ~RenderAPICore();

		/**
		 * Returns the name of the rendering system. 
		 *
		 * @note	Thread safe.
		 */
		virtual const StringID& getName() const = 0;

		/**
		 * Gets the name of the primary shading language used by the rendering system.
		 *
		 * @note	Thread safe.
		 */
		virtual const String& getShadingLanguageName() const = 0;

		/**
		 * Sets a sampler state for the specified texture unit.
		 *
		 * @param[in]	gptype			Determines to which GPU program slot to bind the sampler state.
		 * @param[in]	texUnit			Texture unit index to bind the state to.
		 * @param[in]	samplerState	Sampler state to bind, or null to unbind.
		 *
		 * @see		SamplerState
		 */
		virtual void setSamplerState(GpuProgramType gptype, UINT16 texUnit, const SPtr<SamplerStateCore>& samplerState) = 0;

		/**
		 * Sets a blend state used for all active render targets.
		 *
		 * @param[in]	blendState	Blend state to bind, or null to unbind.
		 *
		 * @see		BlendState
		 */
		virtual void setBlendState(const SPtr<BlendStateCore>& blendState) = 0;

		/**
		 * Sets a state that controls various rasterizer options. 
		 *
		 * @param[in]	rasterizerState		Rasterizer state to bind, or null to unbind.
		 *
		 * @see		RasterizerState
		 */
		virtual void setRasterizerState(const SPtr<RasterizerStateCore>& rasterizerState) = 0;

		/**
		 * Sets a state that controls depth & stencil buffer options.
		 *
		 * @param[in]	depthStencilState		Depth-stencil state to bind, or null to unbind.
		 * @param[in]	stencilRefValue			Stencil reference value to be used for stencil comparisons, if enabled.
		 *
		 * @see		DepthStencilState
		 */
		virtual void setDepthStencilState(const SPtr<DepthStencilStateCore>& depthStencilState, UINT32 stencilRefValue) = 0;

		/**
		 * Binds a texture to the pipeline for the specified GPU program type at the specified slot. If the slot matches 
		 * the one configured in the GPU program the program will be able to access this texture on the GPU.
		 *
		 * @param[in]	gptype			Determines to which GPU program slot to bind the texture.
		 * @param[in]	texUnit			Texture unit index to bind the texture to.
		 * @param[in]	enabled			True to bind the texture at the specified unit, false to unbind.
		 * @param[in]	texPtr			Texture to bind.
		 */
		virtual void setTexture(GpuProgramType gptype, UINT16 texUnit, bool enabled, const SPtr<TextureCore>& texPtr) = 0;

		/**	
		 * Removes a texture at the specified texture unit.
		 *
		 * @param[in]	gptype			Determines at which GPU program slot to unbind the texture.
		 * @param[in]	texUnit			Texture unit index to unbind the texture from.
		 */
		virtual void disableTextureUnit(GpuProgramType gptype, UINT16 texUnit);

		/**	
		 * Binds a texture that can be used for random load/store operations from a GPU program. 
		 *
		 * @param[in]	gptype			Determines to which GPU program slot to bind the texture.
		 * @param[in]	texUnit			Texture unit index to bind the texture to.
		 * @param[in]	enabled			True to bind the texture at the specified unit, false to unbind.
		 * @param[in]	texPtr			Texture to bind.
		 * @param[in]	surface			Determines which surface of the texture to bind.
		 */
		virtual void setLoadStoreTexture(GpuProgramType gptype, UINT16 texUnit, bool enabled,
			const SPtr<TextureCore>& texPtr, const TextureSurface& surface) = 0;

		/**
		 * Binds a buffer that can be used for read or write operations on the GPU.
		 *
		 * @param[in]	gptype		Determines to which GPU program slot to bind the buffer.
		 * @param[in]	unit		GPU program unit index to bind the buffer to.
		 * @param[in]	buffer		Buffer to bind.
		 * @param[in]	loadStore	If true the buffer will be bound with support for unordered reads and writes, otherwise
		 *							it will only be bound for reads.
		 */
		virtual void setBuffer(GpuProgramType gptype, UINT16 unit, const SPtr<GpuBufferCore>& buffer, 
			bool loadStore = false) = 0;

		/**
		 * Signals that rendering for a specific viewport has started. Any draw calls need to be called between beginFrame()
		 * and endFrame(). 
		 */
		virtual void beginFrame() = 0;
		
		/** Ends that rendering to a specific viewport has ended. */
		virtual void endFrame() = 0;

		/**
		 * Sets the active viewport that will be used for all render operations.
		 *
		 * @param[in]	area	Area of the viewport, in normalized ([0,1] range) coordinates.
		 */
		virtual void setViewport(const Rect2& area) = 0;

		/**
		 * Sets the provided vertex buffers starting at the specified source index.	Set buffer to nullptr to clear the 
		 * buffer at the specified index.
		 *
		 * @param[in]	index		Index at which to start binding the vertex buffers.
		 * @param[in]	buffers		A list of buffers to bind to the pipeline.
		 * @param[in]	numBuffers	Number of buffers in the @p buffers list.
		 */
		virtual void setVertexBuffers(UINT32 index, SPtr<VertexBufferCore>* buffers, UINT32 numBuffers) = 0;

		/**
		 * Sets an index buffer to use when drawing. Indices in an index buffer reference vertices in the vertex buffer, 
		 * which increases cache coherency and reduces the size of vertex buffers by eliminating duplicate data.
		 *
		 * @param[in]	buffer	Index buffer to bind, null to unbine.
		 */
		virtual void setIndexBuffer(const SPtr<IndexBufferCore>& buffer) = 0;

		/**
		 * Sets the vertex declaration to use when drawing. Vertex declaration is used to decode contents of a single 
		 * vertex in a vertex buffer.
		 *
		 * @param[in]	vertexDeclaration	Vertex declaration to bind.
		 */
		virtual void setVertexDeclaration(const SPtr<VertexDeclarationCore>& vertexDeclaration) = 0;

		/** 
		 * Sets the draw operation that determines how to interpret the elements of the index or vertex buffers. 
		 *
		 * @param[in]	op	Draw operation to enable.
		 */
		virtual void setDrawOperation(DrawOperationType op) = 0;

		/**
		 * Draw an object based on currently bound GPU programs, vertex declaration and vertex buffers. Draws directly from
		 * the vertex buffer without using indices.
		 *
		 * @param[in]	vertexOffset	Offset into the currently bound vertex buffer to start drawing from.
		 * @param[in]	vertexCount		Number of vertices to draw.
		 * @param[in]	instanceCount	Number of times to draw the provided geometry, each time with an (optionally)
		 *								separate per-instance data.
		 */
		virtual void draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount = 0) = 0;

		/** 
		 * Draw an object based on currently bound GPU programs, vertex declaration, vertex and index buffers. 
		 *
		 * @param[in]	startIndex		Offset into the currently bound index buffer to start drawing from.
		 * @param[in]	indexCount		Number of indices to draw.
		 * @param[in]	vertexOffset	Offset to apply to each vertex index.
		 * @param[in]	vertexCount		Number of vertices to draw.
		 * @param[in]	instanceCount	Number of times to draw the provided geometry, each time with an (optionally)
		 *								separate per-instance data.
		 */
		virtual void drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount, 
			UINT32 instanceCount = 0) = 0;

		/** 
		 * Executes the currently bound compute shader. 
		 *
		 * @param[in]	numGroupsX	Number of groups to start in the X direction. Must be in range [1, 65535].
		 * @param[in]	numGroupsY	Number of groups to start in the Y direction. Must be in range [1, 65535].
		 * @param[in]	numGroupsZ	Number of groups to start in the Z direction. Must be in range [1, 64].
		 */
		virtual void dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY = 1, UINT32 numGroupsZ = 1) = 0;

		/** 
		 * Swap the front and back buffer of the specified render target. 
		 *
		 * @param[in]	target	Render target to perform the buffer swap on.
		 */
		virtual void swapBuffers(const SPtr<RenderTargetCore>& target);

		/**
		 * Gets the capabilities of the render system.
		 *
		 * @note	Thread safe.
		 */
		const RenderAPICapabilities* getCapabilities() const;

		/** Returns information about the driver version. */
		virtual const DriverVersion& getDriverVersion() const;

		/**
		 * Binds the provided GPU program to the pipeline. Any following draw operations will use this program. 
		 *
		 * @param[in]	prg		GPU program to bind. Slot it is bound to is determined by the program type.
		 *
		 * @note	You need to bind at least a vertex and a fragment program in order to draw something.
		 */
		virtual void bindGpuProgram(const SPtr<GpuProgramCore>& prg);

		/**
		 * Binds constant(uniform) GPU program parameters. Caller must ensure these match the previously bound GPU program.
		 *
		 * @param[in]	gptype	GPU program slot to bind the buffer to.
		 * @param[in]	params	Object containing the required constant buffers.
		 */
		virtual void setConstantBuffers(GpuProgramType gptype, const SPtr<GpuParamsCore>& params) = 0;

		/** 
		 * Binds all specified GPU program parameters (textures, buffers, samplers and constant buffers). Caller must
		 * ensure these match the previously bound GPU program. 
		 *
		 * @param[in]	gptype	GPU program slot to bind the buffer to.
		 * @param[in]	params	Object containing the required parameters.
		 */
		virtual void setGpuParams(GpuProgramType gptype, const SPtr<GpuParamsCore>& params);

		/**	
		 * Unbinds a program of a given type. 
		 *
		 * @param[in]	gptype	GPU program slot to unbind the program from.
		 */
		virtual void unbindGpuProgram(GpuProgramType gptype);

		/**	Query if a GPU program of a given type is currently bound. */
		virtual bool isGpuProgramBound(GpuProgramType gptype);

		/**	
		 * Sets up clip planes that will clip drawn geometry on the negative side of the planes. 
		 *
		 * @param[in]	clipPlanes	A list of planes to set, replacing the old ones.
		 */
		virtual void setClipPlanes(const PlaneList& clipPlanes);

		/**	
		 * Adds a new clip plane. All drawn geometry will be clipped to this plane. 
		 *
		 * @param[in]	p	Clip plane to add.
		 */
		virtual void addClipPlane(const Plane& p);

		/**	Clears all clip planes. */
		virtual void resetClipPlanes();

		/**
		 * Allows you to set up a region in which rendering can take place. Coordinates are in pixels. No rendering will be
		 * done to render target pixels outside of the provided region.
		 *
		 * @param[in]	left	Left border of the scissor rectangle, in pixels.
		 * @param[in]	top		Top border of the scissor rectangle, in pixels.
		 * @param[in]	right	Right border of the scissor rectangle, in pixels.
		 * @param[in]	bottom	Bottom border of the scissor rectangle, in pixels.
		 */
		virtual void setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom) = 0;

		/**
		 * Clears the currently active render target.
		 *
		 * @param[in]	buffers		Combination of one or more elements of FrameBufferType denoting which buffers are 
		 *							to be cleared.
		 * @param[in]	color		(optional) The color to clear the color buffer with, if enabled.
		 * @param[in]	depth		(optional) The value to initialize the depth buffer with, if enabled.
		 * @param[in]	stencil		(optional) The value to initialize the stencil buffer with, if enabled.
		 * @param[in]	targetMask	(optional) In case multiple render targets are bound, this allows you to control
		 *							which ones to clear (0x01 first, 0x02 second, 0x04 third, etc., and combinations).
		 */
		virtual void clearRenderTarget(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, 
			UINT16 stencil = 0, UINT8 targetMask = 0xFF) = 0;

		/**
		 * Clears the currently active viewport (meaning it clears just a sub-area of a render-target that is covered by the 
		 * viewport, as opposed to clearRenderTarget() which always clears the entire render target).
		 *
		 * @param[in]	buffers		Combination of one or more elements of FrameBufferType denoting which buffers are to be
		 *							cleared.
		 * @param[in]	color		(optional) The color to clear the color buffer with, if enabled.
		 * @param[in]	depth		(optional) The value to initialize the depth buffer with, if enabled.
		 * @param[in]	stencil		(optional) The value to initialize the stencil buffer with, if enabled.
		 * @param[in]	targetMask	(optional) In case multiple render targets are bound, this allows you to control
		 *							which ones to clear (0x01 first, 0x02 second, 0x04 third, etc., and combinations).
		 */
		virtual void clearViewport(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, 
			UINT16 stencil = 0, UINT8 targetMask = 0xFF) = 0;

		/**
		 * Change the render target into which we want to draw.
		 *
		 * @param[in]	target					Render target to draw to.
		 * @param[in]	readOnlyDepthStencil	If true the caller guarantees he won't write to the depth/stencil buffer 
		 *										(if any was provided). This allows the depth buffer to be bound for depth 
		 *										testing, as well as reading in a shader, at the same time.
		 */
        virtual void setRenderTarget(const SPtr<RenderTargetCore>& target, bool readOnlyDepthStencil = false) = 0;

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
		 * 						populated, the rest will be populated when the method returns.		
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
		 * @note	
		 * Although I'd like otherwise, due to the nature of some render API implementations, you cannot initialize the 
		 * render system without a window.
		 * @note
		 * Sim thread.
		 */
		SPtr<RenderWindow> initialize(const RENDER_WINDOW_DESC& primaryWindowDesc);

		/**
		 * Prepares the initialization of the render API system on the core thread. After the system is prepared a render 
		 * window can be created and initialization finalized.
		 */
		virtual void initializePrepare();

		/**
		 * Finalizes the initialization of the render API system on the core thread. Should be called after the primary 
		 * render window is created.
		 */
		virtual void initializeFinalize(const SPtr<RenderWindowCore>& primaryWindow);

		/**
		 * Shuts down the render API system and cleans up all resources.
		 *
		 * @note	Sim thread.
		 */
		void destroy();

		/** Performs render API system shutdown on the core thread. */
		virtual void destroyCore();

		/** @copydoc setClipPlanes */
		virtual void setClipPlanesImpl(const PlaneList& clipPlanes) = 0;

		/************************************************************************/
		/* 								INTERNAL DATA					       	*/
		/************************************************************************/
	protected:
		friend class RenderAPIManager;

		SPtr<RenderTargetCore> mActiveRenderTarget;

		DriverVersion mDriverVersion;
		CullingMode mCullingMode;
		UINT16 mDisabledTexUnitsFrom;

		bool mVertexProgramBound;
		bool mGeometryProgramBound;
		bool mFragmentProgramBound;
		bool mDomainProgramBound;
		bool mHullProgramBound;
		bool mComputeProgramBound;

		PlaneList mClipPlanes;
		bool mClipPlanesDirty;

		RenderAPICapabilities* mCurrentCapabilities;
		SPtr<VideoModeInfo> mVideoModeInfo;
	};

	/** @} */
}