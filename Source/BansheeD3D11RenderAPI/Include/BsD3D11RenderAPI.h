//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsRenderAPI.h"

namespace BansheeEngine
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/** Implementation of a render system using DirectX 11. Provides abstracted access to various low level DX11 methods. */
	class BS_D3D11_EXPORT D3D11RenderAPI : public RenderAPICore
	{
	public:
		D3D11RenderAPI();
		~D3D11RenderAPI();

		/** @copydoc RenderAPICore::getName */
		const StringID& getName() const override;
		
		/** @copydoc RenderAPICore::getShadingLanguageName */
		const String& getShadingLanguageName() const override;

		/** @copydoc RenderAPICore::setBlendState */
		void setBlendState(const SPtr<BlendStateCore>& blendState) override;

		/** @copydoc RenderAPICore::setRasterizerState */
		void setRasterizerState(const SPtr<RasterizerStateCore>& rasterizerState) override;

		/** @copydoc RenderAPICore::setDepthStencilState */
		void setDepthStencilState(const SPtr<DepthStencilStateCore>& depthStencilState, UINT32 stencilRefValue) override;

		/** @copydoc RenderAPICore::setSamplerState */
		void setSamplerState(GpuProgramType gptype, UINT16 texUnit, const SPtr<SamplerStateCore>& samplerState) override;

		/** @copydoc RenderAPICore::setTexture */
		void setTexture(GpuProgramType gptype, UINT16 texUnit, bool enabled, const SPtr<TextureCore>& texPtr) override;

		/** @copydoc RenderAPICore::setLoadStoreTexture */
		void setLoadStoreTexture(GpuProgramType gptype, UINT16 texUnit, bool enabled, const SPtr<TextureCore>& texPtr,
			const TextureSurface& surface) override;

		/** @copydoc RenderAPICore::setBuffer */
		void setBuffer(GpuProgramType gptype, UINT16 unit, const SPtr<GpuBufferCore>& buffer, bool loadStore = false) override;

		/** @copydoc RenderAPICore::disableTextureUnit */
		void disableTextureUnit(GpuProgramType gptype, UINT16 texUnit) override;

		/** @copydoc RenderAPICore::beginFrame */
		void beginFrame() override;

		/** @copydoc RenderAPICore::endFrame */
		void endFrame() override;

		/** @copydoc RenderAPICore::clearRenderTarget */
		void clearRenderTarget(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0, 
			UINT8 targetMask = 0xFF) override;

		/** @copydoc RenderAPICore::clearViewport */
		void clearViewport(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0,
			UINT8 targetMask = 0xFF) override;

		/** @copydoc RenderAPICore::setRenderTarget */
		void setRenderTarget(const SPtr<RenderTargetCore>& target, bool readOnlyDepthStencil = false) override;

		/** @copydoc RenderAPICore::setViewport */
		void setViewport(const Rect2& area) override;

		/** @copydoc RenderAPICore::setScissorRect */
		void setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom) override;

		/** @copydoc RenderAPICore::setVertexBuffers */
		void setVertexBuffers(UINT32 index, SPtr<VertexBufferCore>* buffers, UINT32 numBuffers) override;

		/** @copydoc RenderAPICore::setIndexBuffer */
		void setIndexBuffer(const SPtr<IndexBufferCore>& buffer) override;

		/** @copydoc RenderAPICore::setVertexDeclaration */
		void setVertexDeclaration(const SPtr<VertexDeclarationCore>& vertexDeclaration) override;

		/** @copydoc RenderAPICore::setDrawOperation */
		void setDrawOperation(DrawOperationType op) override;

		/** @copydoc RenderAPICore::draw */
		void draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount = 0) override;

		/** @copydoc RenderAPICore::drawIndexed */
		void drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount, 
			UINT32 instanceCount = 0) override;

		/** @copydoc RenderAPICore::dispatchCompute */
		void dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY = 1, UINT32 numGroupsZ = 1) override;

		/** @copydoc RenderAPICore::bindGpuProgram */
		void bindGpuProgram(const SPtr<GpuProgramCore>& prg) override;

		/** @copydoc RenderAPICore::unbindGpuProgram */
		void unbindGpuProgram(GpuProgramType gptype) override;

		/** @copydoc RenderAPICore::setConstantBuffers */
		void setConstantBuffers(GpuProgramType gptype, const SPtr<GpuParamsCore>& params) override;
		
		/** @copydoc RenderAPICore::setClipPlanesImpl */
		void setClipPlanesImpl(const PlaneList& clipPlanes) override;

		/** @copydoc RenderAPICore::convertProjectionMatrix */
		void convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest) override;

		/** @copydoc RenderAPICore::getAPIInfo */
		const RenderAPIInfo& getAPIInfo() const override;

		/** @copydoc RenderAPICore::generateParamBlockDesc() */
		GpuParamBlockDesc generateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params) override;

		/************************************************************************/
		/* 				Internal use by DX11 RenderSystem only                  */
		/************************************************************************/

		/**
		 * Determines DXGI multisample settings from the provided parameters.
		 *
		 * @param[in]	multisampleCount	Number of requested samples.
		 * @param[in]	format				Pixel format used by the render target.
		 * @param[out]	outputSampleDesc	Output structure that will contain the requested multisample settings.
		 */
		void determineMultisampleSettings(UINT32 multisampleCount, DXGI_FORMAT format, DXGI_SAMPLE_DESC* outputSampleDesc);

		/**	Returns the main DXGI factory object. */
		IDXGIFactory* getDXGIFactory() const { return mDXGIFactory; }

		/**	Returns the primary DX11 device object. */
		D3D11Device& getPrimaryDevice() const { return *mDevice; }
		
		/**	Returns information describing all available drivers. */
		D3D11DriverList* getDriverList() const { return mDriverList; }

	protected:
		friend class D3D11RenderAPIFactory;

		/** @copydoc RenderAPICore::initializePrepare */
		void initializePrepare() override;

		/** @copydoc RenderAPICore::initializeFinalize */
		void initializeFinalize(const SPtr<RenderWindowCore>& primaryWindow) override;

		/** @copydoc RenderAPICore::destroyCore */
		void destroyCore() override;

		/**
		 * Creates or retrieves a proper input layout depending on the currently set vertex shader and vertex buffer.
		 *
		 * Applies the input layout to the pipeline.
		 */
		void applyInputLayout();

		/**
		 * Recalculates actual viewport dimensions based on currently set viewport normalized dimensions and render target
		 * and applies them for further rendering.
		 */
		void applyViewport();

		/** Creates and populates a set of render system capabilities describing which functionality is available. */
		RenderAPICapabilities* createRenderSystemCapabilities() const;

	private:
		IDXGIFactory* mDXGIFactory;
		D3D11Device* mDevice;

		D3D11DriverList* mDriverList;
		D3D11Driver* mActiveD3DDriver;

		D3D_FEATURE_LEVEL mFeatureLevel;

		D3D11HLSLProgramFactory* mHLSLFactory;
		D3D11InputLayoutManager* mIAManager;

		std::pair<SPtr<TextureCore>, SPtr<TextureView>> mBoundUAVs[D3D11_PS_CS_UAV_REGISTER_COUNT];

		UINT32 mStencilRef;
		Rect2 mViewportNorm;
		D3D11_VIEWPORT mViewport;
		D3D11_RECT mScissorRect;

		SPtr<VertexDeclarationCore> mActiveVertexDeclaration;
		SPtr<D3D11GpuProgramCore> mActiveVertexShader;

		DrawOperationType mActiveDrawOp;
	};

	/** @} */
}