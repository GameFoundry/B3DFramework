//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsRenderAPI.h"
#include "BsGLHardwareBufferManager.h"
#include "GLSL/BsGLSLProgramFactory.h"
#include "Math/BsMatrix4.h"

namespace bs
{
	namespace ct
	{
		class GLCommandBuffer;

		/** @addtogroup GL
		 *  @{
		 */

		/** Implementation of a render system using OpenGL. Provides abstracted access to various low level OpenGL methods. */
		class GLRenderAPI : public RenderAPI
		{
		public:
			GLRenderAPI();
			~GLRenderAPI() = default;

			const StringID& GetName() const override;
			void SetGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetComputePipeline(const SPtr<ComputePipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetGpuParams(const SPtr<GpuParams>& gpuParams, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetViewport(const Rect2& area, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetScissorRect(u32 left, u32 top, u32 right, u32 bottom, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetStencilRef(u32 value, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetVertexBuffers(u32 index, SPtr<VertexBuffer>* buffers, u32 numBuffers, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetIndexBuffer(const SPtr<IndexBuffer>& buffer, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetVertexDeclaration(const SPtr<VertexDeclaration>& vertexDeclaration, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SetDrawOperation(DrawOperationType op, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void DispatchCompute(u32 numGroupsX, u32 numGroupsY = 1, u32 numGroupsZ = 1, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void SwapBuffers(const SPtr<RenderTarget>& target, u32 syncMask = 0xFFFFFFFF) override;
			void SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags = 0, RenderSurfaceMask loadMask = RT_NONE, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void ClearRenderTarget(u32 buffers, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void ClearViewport(u32 buffers, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;
			void AddCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary) override;
			void SubmitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, u32 syncMask = 0xFFFFFFFF) override;
			SPtr<CommandBuffer> GetMainCommandBuffer() const override;
			void ConvertProjectionMatrix(const Matrix4& matrix, Matrix4& dest) override;
			GpuParamBlockDesc GenerateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params) override;

			/************************************************************************/
			/* 				Internal use by OpenGL RenderSystem only                */
			/************************************************************************/

			/**	Query has the main context been initialized. */
			bool IsContextInitializedInternal() const { return mGLInitialised; }

			/**	Returns main context. Caller must ensure the context has been initialized. */
			SPtr<GLContext> GetMainContextInternal() const { return mMainContext; }

			/**	Returns a support object you may use for creating */
			GLSupport* GetGlSupport() const { return mGLSupport; }

		protected:
			void Initialize() override;
			void InitializeWithWindow(const SPtr<RenderWindow>& primaryWindow) override;
			void DestroyCore() override;

			/**	Call before doing a draw operation, this method sets everything up. */
			void BeginDraw();

			/**	Needs to accompany every beginDraw after you are done with a single draw operation. */
			void EndDraw();

			/**	Clear a part of a render target. */
			void ClearArea(u32 buffers, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0, const Rect2I& clearArea = Rect2I::kEmpty, u8 targetMask = 0xFF);

			/**
			 * Changes the currently active texture unit. Any texture related operations will then be performed on this unit.
			 */
			bool ActivateGlTextureUnit(u16 unit);

			/**	Retrieves the active GPU program of the specified type. */
			SPtr<GLSLGpuProgram> GetActiveProgram(GpuProgramType gptype) const;

			/**	Converts framework's blend mode to OpenGL blend mode. */
			GLint GetBlendMode(BlendFactor blendMode) const;

			/**	Converts framework's texture addressing mode to OpenGL texture addressing mode. */
			GLint GetTextureAddressingMode(TextureAddressingMode tam) const;

			/** Gets a combined min/mip filter value usable by OpenGL from the currently set min and mip filters. */
			GLuint GetCombinedMinMipFilter() const;

			/** Returns the OpenGL specific mode used for drawing, depending on the currently set draw operation. */
			GLint GetGlDrawMode() const;

			/**	Creates render system capabilities that specify which features are or aren't supported. */
			void InitCapabilities(RenderAPICapabilities& caps) const;

			/**	Finish initialization by setting up any systems dependant on render systemcapabilities. */
			void InitFromCaps(RenderAPICapabilities* caps);

			/**
			 * Switch the currently used OpenGL context. You will need to re-bind any previously bound values manually
			 * (for example textures, gpu programs and such).
			 */
			void SwitchContext(const SPtr<GLContext>& context, const RenderWindow& window);

			/**
			 * Returns a valid command buffer. Uses the provided buffer if not null. Otherwise returns the default command
			 * buffer.
			 */
			SPtr<GLCommandBuffer> GetCb(const SPtr<CommandBuffer>& buffer);

			/************************************************************************/
			/* 								Sampler states                     		*/
			/************************************************************************/

			/**
			 * Sets the texture addressing mode for a texture unit. This determines how are UV address values outside of [0, 1]
			 * range handled when sampling from texture.
			 */
			void SetTextureAddressingMode(u16 unit, const UVWAddressingMode& uvw);

			/**
			 * Sets the texture border color for a texture unit. Border color determines color returned by the texture sampler
			 * when border addressing mode is used and texture address is outside of [0, 1] range.
			 */
			void SetTextureBorderColor(u16 unit, const Color& color);

			/**
			 * Sets the mipmap bias value for a given texture unit. Bias allows	you to adjust the mipmap selection calculation.
			 * Negative values force a larger mipmap to be used, and positive values smaller. Units are in values of mip levels,
			 * so -1 means use a mipmap one level higher than default.
			 */
			void SetTextureMipmapBias(u16 unit, float bias);

			/**
			 * Sets a valid range for mipmaps (LOD) for a given texture unit. @p min limits the selection of the highest
			 * resolution mipmap (lowest level), and @p max limits the selection of the lowest resolution mipmap (highest
			 * level).
			 */
			void SetTextureMipmapRange(u16 unit, float min, float max);

			/**
			 * Allows you to specify how is the texture bound to the specified texture unit filtered. Different filter types are
			 * used for different situations like magnifying or minifying a texture.
			 */
			void SetTextureFiltering(u16 unit, FilterType ftype, FilterOptions filter);

			/**	Sets anisotropy value for the specified texture unit. */
			void SetTextureAnisotropy(u16 unit, u32 maxAnisotropy);

			/**
			 * Sets the compare mode to use when sampling the texture (anything but "always" implies the use of a shadow
			 * sampler.
			 */
			void SetTextureCompareMode(u16 unit, CompareFunction compare);

			/**	Gets anisotropy value for the specified texture unit. */
			GLfloat GetCurrentAnisotropy(u16 unit);

			/************************************************************************/
			/* 								Blend states                      		*/
			/************************************************************************/

			/**
			 * Sets up blending mode that allows you to combine new pixels with pixels already in the render target.
			 * Final pixel value = (renderTargetPixel * sourceFactor) op (pixel * destFactor).
			 */
			void SetSceneBlending(u32 target, BlendFactor sourceFactor, BlendFactor destFactor, BlendOperation op);

			/**
			 * Sets up blending mode that allows you to combine new pixels with pixels already in the render target.
			 * Allows you to set up separate blend operations for alpha values.
			 *
			 * Final pixel value = (renderTargetPixel * sourceFactor) op (pixel * destFactor). (And the same for alpha)
			 */
			void SetSceneBlending(u32 target, BlendFactor sourceFactor, BlendFactor destFactor, BlendFactor sourceFactorAlpha, BlendFactor destFactorAlpha, BlendOperation op, BlendOperation alphaOp);

			/**
			 * Enable alpha to coverage. Alpha to coverage allows you to perform blending without needing to worry about order
			 * of rendering like regular blending does. It requires multi-sampling to be active in order to work, and you need
			 * to supply an alpha texture that determines object transparency.
			 */
			void SetAlphaToCoverage(bool enabled);

			/**	Enables or disables writing to certain color channels of the render target. */
			void SetColorBufferWriteEnabled(u32 target, bool red, bool green, bool blue, bool alpha);

			/************************************************************************/
			/* 								Rasterizer states                  		*/
			/************************************************************************/

			/**	Sets vertex winding order. Normally you would use this to cull back facing polygons. */
			void SetCullingMode(CullingMode mode);

			/**	Sets the polygon rasterization mode. Determines how are polygons interpreted. */
			void SetPolygonMode(PolygonMode level);

			/**
			 * Sets a depth bias that will offset the depth values of new pixels by the specified amount. Final depth bias value
			 * is a combination of the constant depth bias and slope depth bias. Slope depth bias has more effect the higher
			 * the slope of the rendered polygon.
			 *
			 * @note	This is useful if you want to avoid z fighting for objects at the same or similar depth.
			 */
			void SetDepthBias(float constantBias, float slopeScaleBias);

			/**
			 * Scissor test allows you to mask off rendering in all but a given rectangular area identified by the rectangle
			 * set by setScissorRect().
			 */
			void SetScissorTestEnable(bool enable);

			/**	Enables or disables multisample antialiasing. */
			void SetMultisamplingEnable(bool enable);

			/**	Enables or disables depth clipping (near/fear plane clipping). */
			void SetDepthClipEnable(bool enable);

			/**	Enables or disables antialiased line rendering. */
			void SetAntialiasedLineEnable(bool enable);

			/************************************************************************/
			/* 						Depth stencil state                      		*/
			/************************************************************************/

			/**	Should new pixels perform depth testing using the set depth comparison function before being written. */
			void SetDepthBufferCheckEnabled(bool enabled = true);

			/**	Should new pixels write to the depth buffer. */
			void SetDepthBufferWriteEnabled(bool enabled = true);

			/**
			 * Sets comparison function used for depth testing. Determines how are new and existing pixel values compared - if
			 * comparison function returns true the new pixel is written.
			 */
			void SetDepthBufferFunction(CompareFunction func = CMPF_LESS_EQUAL);

			/**
			 * Turns stencil tests on or off. By default this is disabled. Stencil testing allow you to mask out a part of the
			 * rendered image by using various stencil operations provided.
			 */
			void SetStencilCheckEnabled(bool enabled);

			/**
			 * Allows you to set stencil operations that are performed when stencil test passes or fails.
			 *
			 * @param[in]	stencilFailOp	Operation executed when stencil test fails.
			 * @param[in]	depthFailOp		Operation executed when stencil test succeeds but depth test fails.
			 * @param[in]	passOp			Operation executed when stencil test succeeds and depth test succeeds.
			 * @param[in]	front			Should the stencil operations be applied to front or back facing polygons.
			 */
			void SetStencilBufferOperations(StencilOperation stencilFailOp = SOP_KEEP, StencilOperation depthFailOp = SOP_KEEP, StencilOperation passOp = SOP_KEEP, bool front = true);

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
			void SetStencilBufferFunc(CompareFunction func = CMPF_ALWAYS_PASS, u32 mask = 0xFFFFFFFF, bool ccw = true);

			/**	The bitmask applied to the stencil value before writing it to the stencil buffer. */
			void SetStencilBufferWriteMask(u32 mask = 0xFFFFFFFF);

			/**
			 * Sets a reference values used for stencil buffer comparisons. Actual comparison function and stencil operations
			 * are set by setting the DepthStencilState.
			 */
			void SetStencilRefValue(u32 refValue);

			/************************************************************************/
			/* 							UTILITY METHODS                      		*/
			/************************************************************************/

			/**
			 * Recalculates actual viewport dimensions based on currently set viewport normalized dimensions and render target
			 * and applies them for further rendering.
			 */
			void ApplyViewport();

			/**	Converts the provided matrix m into a representation usable by OpenGL. */
			void MakeGlMatrix(GLfloat gl_matrix[16], const Matrix4& m);

			/**	Converts the engine depth/stencil compare function into OpenGL representation. */
			GLint ConvertCompareFunction(CompareFunction func) const;

			/** Convers the engine stencil operation in OpenGL representation. */
			GLint ConvertStencilOp(StencilOperation op) const;

			/** Notifies the active render target that a rendering command was queued that will potentially change its contents. */
			void NotifyRenderTargetModified();

		private:
			/** Information about a currently bound texture. */
			struct TextureInfo
			{
				GLenum Type = GL_TEXTURE_2D;
			};

			static const u32 kMaxVbCount = 32;

			Rect2 mViewportNorm = Rect2(0.0f, 0.0f, 1.0f, 1.0f);
			u32 mScissorTop = 0;
			u32 mScissorBottom = 720;
			u32 mScissorLeft = 0;
			u32 mScissorRight = 1280;
			u32 mViewportLeft = 0;
			u32 mViewportTop = 0;
			u32 mViewportWidth = 0;
			u32 mViewportHeight = 0;
			bool mScissorEnabled = false;
			bool mScissorRectDirty = false;

			u32 mStencilReadMask = 0xFFFFFFFF;
			u32 mStencilWriteMask = 0xFFFFFFFF;
			u32 mStencilRefValue = 0;
			CompareFunction mStencilCompareFront = CMPF_ALWAYS_PASS;
			CompareFunction mStencilCompareBack = CMPF_ALWAYS_PASS;

			// Last min & mip filtering options, so we can combine them
			FilterOptions mMinFilter;
			FilterOptions mMipFilter;

			// Holds texture type settings for every stage
			u32 mNumTextureUnits = 0;
			TextureInfo* mTextureInfos = nullptr;
			bool mDepthWrite = true;
			bool mColorWrite[BS_MAX_MULTIPLE_RENDER_TARGETS][4];

			GLSupport* mGLSupport;
			bool mGLInitialised;

			GLSLProgramFactory* mGLSLProgramFactory = nullptr;
			GLSLProgramPipelineManager* mProgramPipelineManager = nullptr;

			SPtr<GLSLGpuProgram> mCurrentVertexProgram;
			SPtr<GLSLGpuProgram> mCurrentFragmentProgram;
			SPtr<GLSLGpuProgram> mCurrentGeometryProgram;
			SPtr<GLSLGpuProgram> mCurrentHullProgram;
			SPtr<GLSLGpuProgram> mCurrentDomainProgram;
			SPtr<GLSLGpuProgram> mCurrentComputeProgram;

			const GLSLProgramPipeline* mActivePipeline = nullptr;

			std::array<SPtr<VertexBuffer>, kMaxVbCount> mBoundVertexBuffers;
			SPtr<VertexDeclaration> mBoundVertexDeclaration;
			SPtr<IndexBuffer> mBoundIndexBuffer;
			DrawOperationType mCurrentDrawOperation = DOT_TRIANGLE_LIST;

			SPtr<GLContext> mMainContext;
			SPtr<GLContext> mCurrentContext;
			SPtr<GLCommandBuffer> mMainCommandBuffer;

			bool mDrawCallInProgress = false;

			u16 mActiveTextureUnit = -1;
		};

		/** @} */
	} // namespace ct
} // namespace bs
