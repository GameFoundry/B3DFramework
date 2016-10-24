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

		/** @copydoc RenderAPICore::setGraphicsPipeline */
		void setGraphicsPipeline(const SPtr<GpuPipelineStateCore>& pipelineState, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setComputePipeline */
		void setComputePipeline(const SPtr<GpuProgramCore>& computeProgram,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setGpuParams */
		void setGpuParams(const SPtr<GpuParamsCore>& gpuParams, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::beginFrame */
		void beginFrame(const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::endFrame */
		void endFrame(const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::clearRenderTarget */
		void clearRenderTarget(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0, 
			UINT8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::clearViewport */
		void clearViewport(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0,
			UINT8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setRenderTarget */
		void setRenderTarget(const SPtr<RenderTargetCore>& target, bool readOnlyDepthStencil = false, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setViewport */
		void setViewport(const Rect2& area, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setScissorRect */
		void setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setStencilRef */
		void setStencilRef(UINT32 value, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setVertexBuffers */
		void setVertexBuffers(UINT32 index, SPtr<VertexBufferCore>* buffers, UINT32 numBuffers,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setIndexBuffer */
		void setIndexBuffer(const SPtr<IndexBufferCore>& buffer, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setVertexDeclaration */
		void setVertexDeclaration(const SPtr<VertexDeclarationCore>& vertexDeclaration,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::setDrawOperation */
		void setDrawOperation(DrawOperationType op,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::draw */
		void draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount = 0,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::drawIndexed */
		void drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount, 
			UINT32 instanceCount = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::dispatchCompute */
		void dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY = 1, UINT32 numGroupsZ = 1,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::swapBuffers() */
		void swapBuffers(const SPtr<RenderTargetCore>& target,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPICore::addCommands() */
		void addCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary) override;

		/** @copydoc RenderAPICore::executeCommands() */
		void executeCommands(const SPtr<CommandBuffer>& commandBuffer) override;

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
		void initialize() override;

		/** @copydoc RenderAPICore::initializeFinalize */
		void initializeWithWindow(const SPtr<RenderWindowCore>& primaryWindow) override;

		/** @copydoc RenderAPICore::destroyCore */
		void destroyCore() override;

		/**
		 * Sets a sampler state for the specified texture unit. 
		 *
		 * @param[in]	gptype			Determines to which GPU program slot to bind the sampler state.
		 * @param[in]	texUnit			Texture unit index to bind the state to.
		 * @param[in]	samplerState	Sampler state to bind, or null to unbind.
		 *
		 * @see		SamplerState
		 */
		void setSamplerState(GpuProgramType gptype, UINT16 texUnit, const SPtr<SamplerStateCore>& samplerState);

		/**
		 * Binds a texture to the pipeline for the specified GPU program type at the specified slot. If the slot matches 
		 * the one configured in the GPU program the program will be able to access this texture on the GPU.
		 *
		 * @param[in]	gptype			Determines to which GPU program slot to bind the texture.
		 * @param[in]	texUnit			Texture unit index to bind the texture to.
		 * @param[in]	texture			Texture to bind.
		 */
		void setTexture(GpuProgramType gptype, UINT16 texUnit, const SPtr<TextureCore>& texture);

		/**	
		 * Binds a texture that can be used for random load/store operations from a GPU program. 
		 *
		 * @param[in]	gptype			Determines to which GPU program slot to bind the texture.
		 * @param[in]	texUnit			Texture unit index to bind the texture to.
		 * @param[in]	texture			Texture to bind.
		 * @param[in]	surface			Determines which surface of the texture to bind.
		 */
		void setLoadStoreTexture(GpuProgramType gptype, UINT16 texUnit, const SPtr<TextureCore>& texture, 
			const TextureSurface& surface);

		/**
		 * Binds a buffer that can be used for read or write operations on the GPU.
		 *
		 * @param[in]	gptype			Determines to which GPU program slot to bind the buffer.
		 * @param[in]	unit			GPU program unit index to bind the buffer to.
		 * @param[in]	buffer			Buffer to bind.
		 * @param[in]	loadStore		If true the buffer will be bound with support for unordered reads and writes, 
		 *								otherwise it will only be bound for reads.
		 */
		void setBuffer(GpuProgramType gptype, UINT16 unit, const SPtr<GpuBufferCore>& buffer, 
			bool loadStore = false);

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
		SPtr<D3D11DepthStencilStateCore> mActiveDepthStencilState;

		DrawOperationType mActiveDrawOp;
	};

	/** @} */
}