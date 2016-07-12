//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D9Prerequisites.h"
#include "BsRenderAPI.h"
#include "BsRenderAPICapabilities.h"
#include "BsD3D9Mappings.h"

namespace BansheeEngine 
{
	/** @addtogroup D3D9
	 *  @{
	 */

	/** Implementation of a render system using DirectX 9. Provides abstracted access to various low level DX9 methods. */
	class BS_D3D9_EXPORT D3D9RenderAPI : public RenderAPICore
	{
	public:
		/**	Constructs a new instance of the render system using the provided module instance. */
		D3D9RenderAPI(HINSTANCE hInstance);
		~D3D9RenderAPI();

		/** @copydoc RenderAPICore::getName() */
		const StringID& getName() const override;

		/** @copydoc RenderAPICore::getShadingLanguageName() */
		const String& getShadingLanguageName() const override;

		/** @copydoc RenderAPICore::setRenderTarget() */
		void setRenderTarget(const SPtr<RenderTargetCore>& target, bool readOnlyDepthStencil = false) override;

		/** @copydoc RenderAPICore::bindGpuProgram() */
		void bindGpuProgram(const SPtr<GpuProgramCore>& prg) override;

		/** @copydoc RenderAPICore::unbindGpuProgram() */
		void unbindGpuProgram(GpuProgramType gptype) override;

		/** @copydoc RenderAPICore::setParamBuffer */
		void setParamBuffer(GpuProgramType gptype, UINT32 slot, const SPtr<GpuParamBlockBufferCore>& buffer,
			const GpuParamDesc& paramDesc) override;

		/** @copydoc RenderAPICore::setVertexBuffers() */
		void setVertexBuffers(UINT32 index, SPtr<VertexBufferCore>* buffers, UINT32 numBuffers) override;

		/** @copydoc RenderAPICore::setIndexBuffer() */
		void setIndexBuffer(const SPtr<IndexBufferCore>& buffer) override;

		/** @copydoc RenderAPICore::setVertexDeclaration() */
		void setVertexDeclaration(const SPtr<VertexDeclarationCore>& vertexDeclaration) override;

		/** @copydoc RenderAPICore::setDrawOperation() */
		void setDrawOperation(DrawOperationType op) override;

		/** @copydoc RenderAPICore::setTexture() */
		void setTexture(GpuProgramType gptype, UINT16 texUnit, const SPtr<TextureCore>& texture) override;

		/** @copydoc RenderAPICore::setBuffer */
		void setBuffer(GpuProgramType gptype, UINT16 unit, const SPtr<GpuBufferCore>& buffer, bool loadStore = false) override;

		/** @copydoc RenderAPICore::setLoadStoreTexture() */
		void setLoadStoreTexture(GpuProgramType gptype, UINT16 texUnit, bool enabled, const SPtr<TextureCore>& texture,
			const TextureSurface& surface) override;

		/** @copydoc RenderAPICore::setSamplerState() */
		void setSamplerState(GpuProgramType gptype, UINT16 texUnit, const SPtr<SamplerStateCore>& samplerState) override;

		/** @copydoc RenderAPICore::setBlendState() */
		void setBlendState(const SPtr<BlendStateCore>& blendState) override;

		/** @copydoc RenderAPICore::setRasterizerState() */
		void setRasterizerState(const SPtr<RasterizerStateCore>& rasterizerState) override;

		/** @copydoc RenderAPICore::setDepthStencilState() */
		void setDepthStencilState(const SPtr<DepthStencilStateCore>& depthStencilState, UINT32 stencilRefValue) override;

		/** @copydoc RenderAPICore::setViewport() */
		void setViewport(const Rect2& area) override;

		/** @copydoc RenderAPICore::beginFrame() */
		void beginFrame() override;

		/** @copydoc RenderAPICore::endFrame() */
		void endFrame() override;

		/** @copydoc RenderAPICore::draw() */
		void draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount = 0) override;

		/** @copydoc RenderAPICore::drawIndexed() */
		void drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount,
			UINT32 instanceCount = 0) override;

		/** @copydoc RenderAPICore::dispatchCompute() */
		void dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY = 1, UINT32 numGroupsZ = 1) override;

		/** @copydoc RenderAPICore::setScissorRect() */
		void setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom) override;

		/** @copydoc RenderAPICore::clearRenderTarget() */
		void clearRenderTarget(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0, 
			UINT8 targetMask = 0xFF) override;

		/** @copydoc RenderAPICore::clearViewport() */
		void clearViewport(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0, 
			UINT8 targetMask = 0xFF) override;

		/** @copydoc RenderAPICore::convertProjectionMatrix() */
		void convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest) override;

		/** @copydoc RenderAPICore::getAPIInfo */
		const RenderAPIInfo& getAPIInfo() const override;

		/** @copydoc RenderAPICore::generateParamBlockDesc() */
		GpuParamBlockDesc generateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params) override;

		/************************************************************************/
		/* 				Internal use by DX9 RenderAPI only                   */
		/************************************************************************/

		/**	Returns the resource manager instance. */
		static D3D9ResourceManager* getResourceManager();

		/**	Returns the device manager instance. */
		static D3D9DeviceManager* getDeviceManager();

		/**	Returns the internal DirectX 9 device object. */
		static IDirect3D9* getDirect3D9();

		/**	Returns the number of devices that resources should be created on. */
		static UINT	getResourceCreationDeviceCount();

		/**	Returns DirectX 9 device used for resource creation at the specified index. */
		static IDirect3DDevice9* getResourceCreationDevice(UINT index);

		/**	Returns the currently active DirectX 9 device. */
		static IDirect3DDevice9* getActiveD3D9Device();

		/**
		 * Converts engine multisample options into DirectX 9 specific ones. Also test for multi-sample support on the
		 * device and returns nearest supported type if requested type is not available.
		 *
		 * @param[in]	d3d9Device				Device to check for multisampling.
		 * @param[in]	multisampleCount		Number of requested samples.
		 * @param[in]	d3dPixelFormat			Pixel format used by the render target.
		 * @param[in]	fullscreen				Are we testing multisampling for a full-screen render target.
		 * @param[in]	outMultisampleType		Output parameter containing DirectX type representing valid multisample type.
		 * @param[in]	outMultisampleQuality	Output parameter containing multisample quality.
		 */
		void determineMultisampleSettings(IDirect3DDevice9* d3d9Device, UINT32 multisampleCount, D3DFORMAT d3dPixelFormat,
			bool fullscreen, D3DMULTISAMPLE_TYPE* outMultisampleType, DWORD* outMultisampleQuality) const;

		/**	Register a newly open window with the render system. */
		void registerWindow(RenderWindowCore& renderWindow);

	private:
		friend class D3D9Texture;
		friend class D3D9RenderWindow;
		friend class D3D9Device;
		friend class D3D9TextureManager;
		friend class D3D9TextureCoreManager;
		friend class D3D9DeviceManager;
		friend class D3D9RenderWindowManager;
		friend class D3D9RenderWindowCoreManager;

		/** @copydoc RenderAPICore::initializePrepare */
		void initializePrepare() override;

		/** @copydoc RenderAPICore::initializeFinalize */
		void initializeFinalize(const SPtr<RenderWindowCore>& primaryWindow) override;

		/** @copydoc RenderAPICore::destroy_internal */
		void destroyCore() override;

		/**	Returns a list of available drivers and their properties. */
		D3D9DriverList* getDirect3DDrivers() const;
				
		/**	Sets DirectX 9 render state option. */
		HRESULT setRenderState(D3DRENDERSTATETYPE state, DWORD value);

		/**	Sets DirectX 9 sampler state option for a sampler at the specified index. */
		HRESULT setSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);

		/**	Sets DirectX 9 texture state option for a texture unit at the specified index. */
		HRESULT setTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);

		/**	Set a floating point render state option. */
		HRESULT setFloatRenderState(D3DRENDERSTATETYPE state, float value)
		{
			return setRenderState(state, *((LPDWORD)(&value)));
		}

		/**	Returns currently active anisotropy level for the provided texture unit. */
		DWORD getCurrentAnisotropy(UINT32 unit);

		/**
		 * Updates active render system capabilities. Requires active render window to check certain capabilities.
		 *
		 * @note	Also performs an initialization step when called the first time.
		 */
		RenderAPICapabilities* updateRenderSystemCapabilities(D3D9RenderWindowCore* renderWindow);

		/**	Updates render system capabilities with vertex shader related data. */
        void updateVertexShaderCaps(RenderAPICapabilities* rsc) const;

		/**	Updates render system capabilities with pixel shader related data. */
        void updatePixelShaderCaps(RenderAPICapabilities* rsc) const;

		/** @copydoc RenderAPICore::setClipPlanesImpl */
		void setClipPlanesImpl(const PlaneList& clipPlanes) override;

		/**	Converts a HRESULT error number into an error description. */
		String getErrorDescription(long errorNumber) const;

		/**	Sets a clip plane with the specified index. */
		void setClipPlane(UINT16 index, float A, float B, float C, float D);

		/**	Enables or disables a clip plane at the specified index. */
		void enableClipPlane(UINT16 index, bool enable);

		/**	Returns current module instance. */
		HINSTANCE getInstanceHandle() const { return mhInstance; }

		/** Returns the D3D9 specific mode used for drawing, depending on the currently set draw operation. */
		D3DPRIMITIVETYPE getD3D9PrimitiveType() const;

		/************************************************************************/
		/* 							Sampler states                     			*/
		/************************************************************************/

		/**
		 * Sets the texture addressing mode for a texture unit. This determines how are UV address values outside of [0, 1]
		 * range handled when sampling from texture.
		 */
		void setTextureAddressingMode(UINT16 stage, const UVWAddressingMode& uvw);

		/**
		 * Allows you to specify how is the texture bound to the specified texture unit filtered. Different filter types are
		 * used for different situations like magnifying or minifying a texture.
		 */
		void setTextureFiltering(UINT16 unit, FilterType ftype, FilterOptions filter);

		/**	Sets anisotropy value for the specified texture unit. */
		void setTextureAnisotropy(UINT16 unit, unsigned int maxAnisotropy);

		/**
		 * Sets the texture border color for a texture unit. Border color determines color returned by the texture sampler
		 * when border addressing mode is used and texture address is outside of [0, 1] range.
		 */
		void setTextureBorderColor(UINT16 stage, const Color& color);

		/**
		 * Sets the mipmap bias value for a given texture unit. Bias allows you to adjust the mipmap selection calculation.
		 * Negative values force a larger mipmap to be used, and positive values smaller. Units are in values of mip levels,
		 * so -1 means use a mipmap one level higher than default.
		 */
		void setTextureMipmapBias(UINT16 unit, float bias);

		/************************************************************************/
		/* 								Blend states                      		*/
		/************************************************************************/

		/**
		 * Sets up blending mode that allows you to combine new pixels with pixels already in the render target. Final pixel
		 * value = (renderTargetPixel * sourceFactor) op (pixel * destFactor).
		 */
		void setSceneBlending(BlendFactor sourceFactor, BlendFactor destFactor, BlendOperation op);

		/**
		 * Sets up blending mode that allows you to combine new pixels with pixels already in the render target. Allows you
		 * to set up separate blend operations for alpha values.
		 *	
		 *	Final pixel value = (renderTargetPixel * sourceFactor) op (pixel * destFactor). (And the same for alpha)
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

		/** Sets vertex winding order. Normally you would use this to cull back facing polygons. */
		void setCullingMode(CullingMode mode);

		/**	Sets the polygon rasterization mode. Determines how are polygons interpreted. */
		void setPolygonMode(PolygonMode level);

		/**
		 * Sets a depth bias that will offset the depth values of new pixels by the specified amount. Final depth bias value
		 * is a combination of the constant depth bias and slope depth bias. Slope depth bias has more effect the higher the
		 * slope of the rendered polygon.
		 *
		 * @note	This is useful if you want to avoid z fighting for objects at the same or similar depth.
		 */
		void setDepthBias(float constantBias, float slopeScaleBias);

		/**
		 * Scissor test allows you to mask off rendering in all but a given rectangular area identified by the rectangle
		 * set by setScissorRect().
		 */
		void setScissorTestEnable(bool enable);

		/**
		 * Only applies when rendering to a multisample render target. If disabled all of the samples will be taken from the
		 * center of the pixel, effectively making the image aliased. Default value is true where samples are picked
		 * randomly within the pixel.
		 */
		void setMultisampleAntialiasEnable(bool enable);

		/**	
		 * Only applies when rendering to a non-multisample render target. If enabled, lines will be antialiased. Default
		 * state is disabled.
		 */
		void setAntialiasedLineEnable(bool enable);

		/************************************************************************/
		/* 						Depth stencil state                      		*/
		/************************************************************************/
		
		/** Should new pixels perform depth testing using the set depth comparison function before being written. */
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
			bool ccw = true);

		/**
		 * Sets a stencil buffer comparison function. The result of this will cause one of 3 actions depending on whether
		 * the test fails, succeeds but with the depth buffer check still failing, or succeeds with the depth buffer check
		 * passing too.
		 *
		 * @param[in] func	Comparison function that determines whether a stencil test fails or passes. Reference value
		 *					gets compared to the value already in the buffer using this function.
		 * @param[in] ccw	If set to true, the stencil operations will be applied to counterclockwise
		 *					faces. Otherwise they will be applied to clockwise faces.
		 */
		void setStencilBufferFunc(CompareFunction func = CMPF_ALWAYS_PASS, bool ccw = true);

		/** The bitmask applied to both the stencil value and the reference value before comparison. */
		void setStencilBufferReadMask(UINT32 mask = 0xFFFFFFFF);

		/**	The bitmask applied to the stencil value before writing it to the stencil buffer. */
		void setStencilBufferWriteMask(UINT32 mask = 0xFFFFFFFF);

		/**
		 * Sets a reference values used for stencil buffer comparisons. Actual comparison function and stencil operations
		 * are set by setting the DepthStencilState.
		 */
		void setStencilRefValue(UINT32 refValue);

		/**
		 * Clears an area of the currently active render target.
		 *
		 * @param[in]	buffers			Combination of one or more elements of FrameBufferType denoting which buffers are
		 *								to be cleared.
		 * @param[in]	color			(optional) The color to clear the color buffer with, if enabled.
		 * @param[in]	depth			(optional) The value to initialize the depth buffer with, if enabled.
		 * @param[in]	stencil			(optional) The value to initialize the stencil buffer with, if enabled.
		 * @param[in]	clearArea		(optional) Area in pixels to clear.
		 */
		void clearArea(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0, const Rect2I& clearArea = Rect2I::EMPTY);

		/**
		 * Recalculates actual viewport dimensions based on currently set viewport normalized dimensions and render target
		 * and applies them for further rendering.
		 */
		void applyViewport();

		/**	Triggered when device has been lost. */
		void notifyOnDeviceLost(D3D9Device* device);

		/**	Triggered when device is being reset. */
		void notifyOnDeviceReset(D3D9Device* device);

	private:
		/**	Holds texture unit settings. */
		struct sD3DTextureStageDesc
		{
			D3D9Mappings::D3DTexType texType;
			size_t coordIndex;
			IDirect3DBaseTexture9 *pTex;
			IDirect3DBaseTexture9 *pVertexTex;
		};

		static D3D9RenderAPI* msD3D9RenderSystem;

		IDirect3D9*	mpD3D;
		D3D9HLSLProgramFactory* mHLSLProgramFactory;
		D3D9ResourceManager* mResourceManager;
		D3D9DeviceManager* mDeviceManager;

		mutable D3D9DriverList* mDriverList;
		D3D9Driver* mActiveD3DDriver;

		UINT32 mNumTexStages;
		sD3DTextureStageDesc* mTexStageDesc;

		bool mIsFrameInProgress;
		bool mRestoreFrameOnReset;

		HINSTANCE mhInstance;

		Rect2 mViewportNorm;
		UINT32 mViewportLeft, mViewportTop, mViewportWidth, mViewportHeight;
		RECT mScissorRect;

		DrawOperationType mCurrentDrawOperation;
	};

	/** @} */
}