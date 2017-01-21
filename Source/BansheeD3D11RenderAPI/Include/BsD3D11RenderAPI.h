//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsRenderAPI.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/** Implementation of a render system using DirectX 11. Provides abstracted access to various low level DX11 methods. */
	class BS_D3D11_EXPORT D3D11RenderAPI : public RenderAPI
	{
	public:
		D3D11RenderAPI();
		~D3D11RenderAPI();

		/** @copydoc RenderAPI::getName */
		const StringID& getName() const override;
		
		/** @copydoc RenderAPI::getShadingLanguageName */
		const String& getShadingLanguageName() const override;

		/** @copydoc RenderAPI::setGraphicsPipeline */
		void setGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setComputePipeline */
		void setComputePipeline(const SPtr<ComputePipelineState>& pipelineState,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setGpuParams */
		void setGpuParams(const SPtr<GpuParams>& gpuParams, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::clearRenderTarget */
		void clearRenderTarget(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0, 
			UINT8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::clearViewport */
		void clearViewport(UINT32 buffers, const Color& color = Color::Black, float depth = 1.0f, UINT16 stencil = 0,
			UINT8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setRenderTarget */
		void setRenderTarget(const SPtr<RenderTarget>& target, bool readOnlyDepthStencil = false, 
			RenderSurfaceMask loadMask = RT_NONE, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setViewport */
		void setViewport(const Rect2& area, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setScissorRect */
		void setScissorRect(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setStencilRef */
		void setStencilRef(UINT32 value, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setVertexBuffers */
		void setVertexBuffers(UINT32 index, SPtr<VertexBuffer>* buffers, UINT32 numBuffers,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setIndexBuffer */
		void setIndexBuffer(const SPtr<IndexBuffer>& buffer, 
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setVertexDeclaration */
		void setVertexDeclaration(const SPtr<VertexDeclaration>& vertexDeclaration,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::setDrawOperation */
		void setDrawOperation(DrawOperationType op,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::draw */
		void draw(UINT32 vertexOffset, UINT32 vertexCount, UINT32 instanceCount = 0,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::drawIndexed */
		void drawIndexed(UINT32 startIndex, UINT32 indexCount, UINT32 vertexOffset, UINT32 vertexCount, 
			UINT32 instanceCount = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::dispatchCompute */
		void dispatchCompute(UINT32 numGroupsX, UINT32 numGroupsY = 1, UINT32 numGroupsZ = 1,
			const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

		/** @copydoc RenderAPI::swapBuffers() */
		void swapBuffers(const SPtr<RenderTarget>& target, UINT32 syncMask = 0xFFFFFFFF) override;

		/** @copydoc RenderAPI::addCommands() */
		void addCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary) override;

		/** @copydoc RenderAPI::submitCommandBuffer() */
		void submitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, UINT32 syncMask = 0xFFFFFFFF) override;

		/** @copydoc RenderAPI::convertProjectionMatrix */
		void convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest) override;

		/** @copydoc RenderAPI::getAPIInfo */
		const RenderAPIInfo& getAPIInfo() const override;

		/** @copydoc RenderAPI::generateParamBlockDesc() */
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

		/** @copydoc RenderAPI::initialize */
		void initialize() override;

		/** @copydoc RenderAPI::initializeWithWindow */
		void initializeWithWindow(const SPtr<RenderWindow>& primaryWindow) override;

		/** @copydoc RenderAPI::destroyCore */
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
		void setSamplerState(GpuProgramType gptype, UINT16 texUnit, const SPtr<SamplerState>& samplerState);

		/**
		 * Binds a texture to the pipeline for the specified GPU program type at the specified slot. If the slot matches 
		 * the one configured in the GPU program the program will be able to access this texture on the GPU.
		 *
		 * @param[in]	gptype			Determines to which GPU program slot to bind the texture.
		 * @param[in]	texUnit			Texture unit index to bind the texture to.
		 * @param[in]	texture			Texture to bind.
		 */
		void setTexture(GpuProgramType gptype, UINT16 texUnit, const SPtr<Texture>& texture);

		/**	
		 * Binds a texture that can be used for random load/store operations from a GPU program. 
		 *
		 * @param[in]	gptype			Determines to which GPU program slot to bind the texture.
		 * @param[in]	texUnit			Texture unit index to bind the texture to.
		 * @param[in]	texture			Texture to bind.
		 * @param[in]	surface			Determines which surface of the texture to bind.
		 */
		void setLoadStoreTexture(GpuProgramType gptype, UINT16 texUnit, const SPtr<Texture>& texture, 
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
		void setBuffer(GpuProgramType gptype, UINT16 unit, const SPtr<GpuBuffer>& buffer, 
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
		void initCapabilites(IDXGIAdapter* adapter, RenderAPICapabilities& caps) const;

	private:
		IDXGIFactory* mDXGIFactory;
		D3D11Device* mDevice;

		D3D11DriverList* mDriverList;
		D3D11Driver* mActiveD3DDriver;

		D3D_FEATURE_LEVEL mFeatureLevel;

		D3D11HLSLProgramFactory* mHLSLFactory;
		D3D11InputLayoutManager* mIAManager;

		bool mPSUAVsBound;
		bool mCSUAVsBound;

		UINT32 mStencilRef;
		Rect2 mViewportNorm;
		D3D11_VIEWPORT mViewport;
		D3D11_RECT mScissorRect;

		SPtr<VertexDeclaration> mActiveVertexDeclaration;
		SPtr<D3D11GpuProgram> mActiveVertexShader;
		SPtr<D3D11DepthStencilState> mActiveDepthStencilState;

		DrawOperationType mActiveDrawOp;
	};

	/** @} */
}}