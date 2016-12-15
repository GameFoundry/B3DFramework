//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsRenderAPI.h"
#include "BsGLHardwareBufferManager.h"
#include "BsGLSLProgramFactory.h"
#include "BsMatrix4.h"

namespace bs 
{
	/** @addtogroup GL
	 *  @{
	 */

	/** Implementation of a render system using OpenGL. Provides abstracted access to various low level OpenGL methods. */
    class BS_RSGL_EXPORT GLRenderAPI : public RenderAPICore
    {
    public:
        GLRenderAPI();
        ~GLRenderAPI();

		/** @copydoc RenderAPICore::getName() */
		const StringID& getName() const override;

		/** @copydoc RenderAPICore::getShadingLanguageName() */
		const String& getShadingLanguageName() const override;

		/** @copydoc RenderAPICore::setGraphicsPipeline */
		void setGraphicsPipeline(const SPtr<GraphicsPipelineStateCore>& pipelineState,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setComputePipeline */
		void setComputePipeline(const SPtr<ComputePipelineStateCore>& pipelineState,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setGpuParams() */
		void setGpuParams(const SPtr<GpuParamsCore>& gpuParams,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setViewport() */
		void setViewport(const Rect2& area, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setScissorRect() */
		void setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setStencilRef */
		void setStencilRef(UINT32 value, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setVertexBuffers() */
		void setVertexBuffers(UINT32 index, SPtr<VertexBufferCore>* buffers, UINT32 numBuffers,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setIndexBuffer() */
		void setIndexBuffer(const SPtr<IndexBufferCore>& buffer,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setVertexDeclaration() */
		void setVertexDeclaration(const SPtr<VertexDeclarationCore>& vertexDeclaration,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setDrawOperation() */
		void setDrawOperation(DrawOperationType op, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::draw() */
		void draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount = 0,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::drawIndexed() */
		void drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount
			, UINT32 instanceCount = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::dispatchCompute() */
		void dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY = 1, UINT32 numGroupsZ = 1, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::swapBuffers() */
		void swapBuffers(const SPtr<RenderTargetCore>& target, UINT32 syncMask = 0xFFFFFFFF) override;

		/** @copydoc RenderAPICore::setRenderTarget() */
		void setRenderTarget(const SPtr<RenderTargetCore>& target, bool readOnlyDepthStencil = false, 
			RenderSurfaceMask loadMask = RT_NONE, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::clearRenderTarget() */
		void clearRenderTarget(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0, 
			UINT8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::clearViewport() */
		void clearViewport(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0, 
			UINT8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::addCommands() */
		void addCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary) override;

		/** @copydoc RenderAPICore::submitCommandBuffer() */
		void submitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, UINT32 syncMask = 0xFFFFFFFF) override;

		/** @copydoc RenderAPICore::convertProjectionMatrix() */
		void convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest) override;

		/** @copydoc RenderAPICore::getAPIInfo */
		const RenderAPIInfo& getAPIInfo() const override;

		/** @copydoc RenderAPICore::generateParamBlockDesc() */
		GpuParamBlockDesc generateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params) override;

		/************************************************************************/
		/* 				Internal use by OpenGL RenderSystem only                */
		/************************************************************************/

		/**	Query has the main context been initialized. */
		bool _isContextInitialized() const { return mGLInitialised; }

		/**	Returns main context. Caller must ensure the context has been initialized. */
		SPtr<GLContext> getMainContext() const { return mMainContext; } 

		/**	Returns a support object you may use for creating */
		GLSupport* getGLSupport() const { return mGLSupport; }

	protected:
		/** @copydoc RenderAPICore::initialize */
		void initialize() override;

		/** @copydoc RenderAPICore::initializeWithWindow */
		void initializeWithWindow(const SPtr<RenderWindowCore>& primaryWindow) override;

		/** @copydoc RenderAPICore::destroyCore */
		void destroyCore() override;

		/**	Call before doing a draw operation, this method sets everything up. */
		void beginDraw();

		/**	Needs to accompany every beginDraw after you are done with a single draw operation. */
		void endDraw();

		/**	Clear a part of a render target. */
		void clearArea(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0, 
			const Rect2I& clearArea = Rect2I::EMPTY, UINT8 targetMask = 0xFF);

		/** 
		 * Changes the currently active texture unit. Any texture related operations will then be performed on this unit. 
		 */
		bool activateGLTextureUnit(UINT16 unit);

		/**	Retrieves the active GPU program of the specified type. */
		SPtr<GLSLGpuProgramCore> getActiveProgram(GpuProgramType gptype) const;

		/**	Converts Banshee blend mode to OpenGL blend mode. */
		GLint getBlendMode(BlendFactor blendMode) const;

		/**	Converts Banshee texture addressing mode to OpenGL texture addressing mode. */
		GLint getTextureAddressingMode(TextureAddressingMode tam) const;

		/** Gets a combined min/mip filter value usable by OpenGL from the currently set min and mip filters. */
		GLuint getCombinedMinMipFilter() const;

		/** Returns the OpenGL specific mode used for drawing, depending on the currently set draw operation. */
		GLint getGLDrawMode() const;

		/**	Creates render system capabilities that specify which features are or aren't supported. */
		void initCapabilities(RenderAPICapabilities& caps) const;

		/**	Finish initialization by setting up any systems dependant on render systemcapabilities. */
		void initFromCaps(RenderAPICapabilities* caps);

		/**
		 * Switch the currently used OpenGL context. You will need to re-bind any previously bound values manually
		 * (for example textures, gpu programs and such).
		 */
		void switchContext(const SPtr<GLContext>& context);

		/************************************************************************/
		/* 								Sampler states                     		*/
		/************************************************************************/

		/**
		 * Sets the texture addressing mode for a texture unit. This determines how are UV address values outside of [0, 1]
		 * range handled when sampling from texture.
		 */
        void setTextureAddressingMode(UINT16 unit, const UVWAddressingMode& uvw);

		/**
		 * Sets the texture border color for a texture unit. Border color determines color returned by the texture sampler
		 * when border addressing mode is used and texture address is outside of [0, 1] range.
		 */
        void setTextureBorderColor(UINT16 unit, const Color& color);

		/**
		 * Sets the mipmap bias value for a given texture unit. Bias allows	you to adjust the mipmap selection calculation.
		 * Negative values force a larger mipmap to be used, and positive values smaller. Units are in values of mip levels,
		 * so -1 means use a mipmap one level higher than default.
		 */
		void setTextureMipmapBias(UINT16 unit, float bias);

		/**
		 * Allows you to specify how is the texture bound to the specified texture unit filtered. Different filter types are
		 * used for different situations like magnifying or minifying a texture.
		 */
        void setTextureFiltering(UINT16 unit, FilterType ftype, FilterOptions filter);

		/**	Sets anisotropy value for the specified texture unit. */
		void setTextureAnisotropy(UINT16 unit, UINT32 maxAnisotropy);

		/**	Gets anisotropy value for the specified texture unit. */
		GLfloat getCurrentAnisotropy(UINT16 unit);

		/************************************************************************/
		/* 								Blend states                      		*/
		/************************************************************************/

		/**
		 * Sets up blending mode that allows you to combine new pixels with pixels already in the render target.
		 * Final pixel value = (renderTargetPixel * sourceFactor) op (pixel * destFactor).
		 */
		void setSceneBlending(BlendFactor sourceFactor, BlendFactor destFactor, BlendOperation op);

		/**
		 * Sets up blending mode that allows you to combine new pixels with pixels already in the render target.
		 * Allows you to set up separate blend operations for alpha values.
		 *	
		 * Final pixel value = (renderTargetPixel * sourceFactor) op (pixel * destFactor). (And the same for alpha)
		 */
		void setSceneBlending(BlendFactor sourceFactor, BlendFactor destFactor, BlendFactor sourceFactorAlpha, 
			BlendFactor destFactorAlpha, BlendOperation op, BlendOperation alphaOp);

		/**
		 * Sets alpha test that allows you to reject pixels that fail the comparison function versus the provided reference
		 * value.
		 */
		void setAlphaTest(CompareFunction func, unsigned char value);

		/**
		 * Enable alpha to coverage. Alpha to coverage allows you to perform blending without needing to worry about order
		 * of rendering like regular blending does. It requires multi-sampling to be active in order to work, and you need
		 * to supply an alpha texture that determines object transparency.
		 */
		void setAlphaToCoverage(bool enabled);

		/**	Enables or disables writing to certain color channels of the render target. */
		void setColorBufferWriteEnabled(bool red, bool green, bool blue, bool alpha);

		/************************************************************************/
		/* 								Rasterizer states                  		*/
		/************************************************************************/

		/**	Sets vertex winding order. Normally you would use this to cull back facing polygons. */
		void setCullingMode(CullingMode mode);

		/**	Sets the polygon rasterization mode. Determines how are polygons interpreted. */
		void setPolygonMode(PolygonMode level);

		/**
		 * Sets a depth bias that will offset the depth values of new pixels by the specified amount. Final depth bias value
		 * is a combination of the constant depth bias and slope depth bias. Slope depth bias has more effect the higher
		 * the slope of the rendered polygon.
		 *
		 * @note	This is useful if you want to avoid z fighting for objects at the same or similar depth.
		 */
		void setDepthBias(float constantBias, float slopeScaleBias);

		/**
		 * Scissor test allows you to mask off rendering in all but a given rectangular area identified by the rectangle
		 * set by setScissorRect().
		 */
		void setScissorTestEnable(bool enable);

		/**	Enables or disables multisample antialiasing. */
		void setMultisamplingEnable(bool enable);

		/**	Enables or disables depth clipping (near/fear plane clipping). */
		void setDepthClipEnable(bool enable);

		/**	Enables or disables antialiased line rendering. */
		void setAntialiasedLineEnable(bool enable);

		/************************************************************************/
		/* 						Depth stencil state                      		*/
		/************************************************************************/
		
		/**	Should new pixels perform depth testing using the set depth comparison function before being written. */
		void setDepthBufferCheckEnabled(bool enabled = true);

		/**	Should new pixels write to the depth buffer. */
		void setDepthBufferWriteEnabled(bool enabled = true);

		/**
		 * Sets comparison function used for depth testing. Determines how are new and existing pixel values compared - if
		 * comparison function returns true the new pixel is written.
		 */
		void setDepthBufferFunction(CompareFunction func = CMPF_LESS_EQUAL);

		/**
		 * Turns stencil tests on or off. By default this is disabled. Stencil testing allow you to mask out a part of the
		 * rendered image by using various stencil operations provided.
		 */
		void setStencilCheckEnabled(bool enabled);

		/**
		 * Allows you to set stencil operations that are performed when stencil test passes or fails.
		 *
		 * @param[in]	stencilFailOp	Operation executed when stencil test fails.
		 * @param[in]	depthFailOp		Operation executed when stencil test succeeds but depth test fails.
		 * @param[in]	passOp			Operation executed when stencil test succeeds and depth test succeeds.
		 * @param[in]	front			Should the stencil operations be applied to front or back facing polygons.
		 */
		void setStencilBufferOperations(StencilOperation stencilFailOp = SOP_KEEP,
			StencilOperation depthFailOp = SOP_KEEP, StencilOperation passOp = SOP_KEEP,
			bool front = true);

		/**
		 * Sets a stencil buffer comparison function. The result of this will cause one of 3 actions depending on whether
		 * the test fails, succeeds but with the depth buffer check still failing, or succeeds with the depth buffer check
		 * passing too.
		 *
		 * @param[in] func	Comparison function that determines whether a stencil test fails or passes. Reference value
		 *					gets compared to the value already in the buffer using this function.
		 * @param[in] mask	The bitmask applied to both the stencil value and the reference value
		 *					before comparison
		 * @param[in] ccw	If set to true, the stencil operations will be applied to counterclockwise
		 *					faces. Otherwise they will be applied to clockwise faces.
		 */
		void setStencilBufferFunc(CompareFunction func = CMPF_ALWAYS_PASS, UINT32 mask = 0xFFFFFFFF, bool ccw = true);

		/**	The bitmask applied to the stencil value before writing it to the stencil buffer. */
		void setStencilBufferWriteMask(UINT32 mask = 0xFFFFFFFF);

		/**
		 * Sets a reference values used for stencil buffer comparisons. Actual comparison function and stencil operations
		 * are set by setting the DepthStencilState.
		 */
		void setStencilRefValue(UINT32 refValue);

		/************************************************************************/
		/* 							UTILITY METHODS                      		*/
		/************************************************************************/

		/**
		 * Recalculates actual viewport dimensions based on currently set viewport normalized dimensions and render target
		 * and applies them for further rendering.
		 */
		void applyViewport();

		/**	Converts the provided matrix m into a representation usable by OpenGL. */
		void makeGLMatrix(GLfloat gl_matrix[16], const Matrix4& m);

		/**	Converts the engine depth/stencil compare function into OpenGL representation. */
		GLint convertCompareFunction(CompareFunction func) const;

		/**
		 * Convers the engine stencil operation in OpenGL representation. Optionally inverts the operation (increment
		 * becomes decrement, etc.).
		 */
		GLint convertStencilOp(StencilOperation op, bool invert = false) const;

		/**	Checks if there are any OpenGL errors and prints them to the log. */
		bool checkForErrors() const;

	private:
		/** Information about a currently bound texture. */
		struct TextureInfo
		{
			GLenum type;
		};

		static const UINT32 MAX_VB_COUNT = 32;

		Rect2 mViewportNorm;
		UINT32 mScissorTop, mScissorBottom, mScissorLeft, mScissorRight;
		UINT32 mViewportLeft, mViewportTop, mViewportWidth, mViewportHeight;
		bool mScissorEnabled;

		UINT32 mStencilReadMask;
		UINT32 mStencilWriteMask;
		UINT32 mStencilRefValue;
		CompareFunction mStencilCompareFront;
		CompareFunction mStencilCompareBack;

		// Last min & mip filtering options, so we can combine them
		FilterOptions mMinFilter;
		FilterOptions mMipFilter;

		// Holds texture type settings for every stage
		UINT32 mNumTextureUnits;
		TextureInfo* mTextureInfos;
		bool mDepthWrite;
		bool mColorWrite[4];

		GLSupport* mGLSupport;
		bool mGLInitialised;

		GLSLProgramFactory* mGLSLProgramFactory;
		GLSLProgramPipelineManager* mProgramPipelineManager;

		SPtr<GLSLGpuProgramCore> mCurrentVertexProgram;
		SPtr<GLSLGpuProgramCore> mCurrentFragmentProgram;
		SPtr<GLSLGpuProgramCore> mCurrentGeometryProgram;
		SPtr<GLSLGpuProgramCore> mCurrentHullProgram;
		SPtr<GLSLGpuProgramCore> mCurrentDomainProgram;
		SPtr<GLSLGpuProgramCore> mCurrentComputeProgram;

		const GLSLProgramPipeline* mActivePipeline;

		std::array<SPtr<VertexBufferCore>, MAX_VB_COUNT> mBoundVertexBuffers;
		SPtr<VertexDeclarationCore> mBoundVertexDeclaration;
		SPtr<IndexBufferCore> mBoundIndexBuffer;
		DrawOperationType mCurrentDrawOperation;

		SPtr<GLContext> mMainContext;
		SPtr<GLContext> mCurrentContext;

		bool mDrawCallInProgress;

		UINT16 mActiveTextureUnit;
    };

	/** @} */
}
