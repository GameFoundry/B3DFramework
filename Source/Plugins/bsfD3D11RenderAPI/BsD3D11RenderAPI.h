//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsRenderAPI.h"

namespace bs
{
	namespace ct
	{
		class D3D11CommandBuffer;

		/** @addtogroup D3D11
		 *  @{
		 */

		/** Implementation of a render system using DirectX 11. Provides abstracted access to various low level DX11 methods. */
		class D3D11RenderAPI : public RenderAPI
		{
		public:
			D3D11RenderAPI() = default;
			~D3D11RenderAPI() = default;

			/** @copydoc RenderAPI::getName */
			const StringID& GetName() const;

			/** @copydoc RenderAPI::setGraphicsPipeline */
			void SetGraphicsPipeline(const SPtr<GraphicsPipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/** @copydoc RenderAPI::setComputePipeline */
			void SetComputePipeline(const SPtr<ComputePipelineState>& pipelineState, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/** @copydoc RenderAPI::setGpuParams */
			void SetGpuParams(const SPtr<GpuParams>& gpuParams, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/** @copydoc RenderAPI::clearRenderTarget */
			void ClearRenderTarget(u32 buffers, const Color& color = Color::Black, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/** @copydoc RenderAPI::clearViewport */
			void ClearViewport(u32 buffers, const Color& color = Color::Black, float depth = 1.0f, u16 stencil = 0, u8 targetMask = 0xFF, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/** @copydoc RenderAPI::setRenderTarget */
			void SetRenderTarget(const SPtr<RenderTarget>& target, u32 readOnlyFlags, RenderSurfaceMask loadMask = RT_NONE, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

			/** @copydoc RenderAPI::setViewport */
			void SetViewport(const Rect2& area, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/** @copydoc RenderAPI::setScissorRect */
			void SetScissorRect(u32 left, u32 top, u32 right, u32 bottom, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/** @copydoc RenderAPI::setStencilRef */
			void SetStencilRef(u32 value, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/** @copydoc RenderAPI::setVertexBuffers */
			void SetVertexBuffers(u32 index, SPtr<VertexBuffer>* buffers, u32 numBuffers, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/** @copydoc RenderAPI::setIndexBuffer */
			void SetIndexBuffer(const SPtr<IndexBuffer>& buffer, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

			/** @copydoc RenderAPI::setVertexDeclaration */
			void SetVertexDeclaration(const SPtr<VertexDeclaration>& vertexDeclaration, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/** @copydoc RenderAPI::setDrawOperation */
			void SetDrawOperation(DrawOperationType op, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/** @copydoc RenderAPI::draw */
			void Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/** @copydoc RenderAPI::drawIndexed */
			void DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/** @copydoc RenderAPI::dispatchCompute */
			void DispatchCompute(u32 numGroupsX, u32 numGroupsY = 1, u32 numGroupsZ = 1, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/** @copydoc RenderAPI::swapBuffers() */
			void SwapBuffers(const SPtr<RenderTarget>& target, u32 syncMask = 0xFFFFFFFF);

			/** @copydoc RenderAPI::addCommands() */
			void AddCommands(const SPtr<CommandBuffer>& commandBuffer, const SPtr<CommandBuffer>& secondary);

			/** @copydoc RenderAPI::submitCommandBuffer() */
			void SubmitCommandBuffer(const SPtr<CommandBuffer>& commandBuffer, u32 syncMask = 0xFFFFFFFF);

			/** @copydoc RenderAPI::getMainCommandBuffer() */
			SPtr<CommandBuffer> GetMainCommandBuffer() const override;

			/** @copydoc RenderAPI::convertProjectionMatrix */
			void ConvertProjectionMatrix(const Matrix4& matrix, Matrix4& dest) override;

			/** @copydoc RenderAPI::generateParamBlockDesc() */
			GpuParamBlockDesc GenerateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params);

			/************************************************************************/
			/* 				Internal use by DX11 backend only						*/
			/************************************************************************/

			/**
			 * Determines DXGI multisample settings from the provided parameters.
			 *
			 * @param[in]	multisampleCount	Number of requested samples.
			 * @param[in]	format				Pixel format used by the render target.
			 * @param[out]	outputSampleDesc	Output structure that will contain the requested multisample settings.
			 */
			void DetermineMultisampleSettings(u32 multisampleCount, DXGI_FORMAT format, DXGI_SAMPLE_DESC* outputSampleDesc);

			/**	Returns the main DXGI factory object. */
			IDXGIFactory1* GetDxgiFactory() const { return mDXGIFactory; }

			/**	Returns the primary DX11 device object. */
			D3D11Device& GetPrimaryDevice() const { return *mDevice; }

			/**	Returns information describing all available drivers. */
			D3D11DriverList* GetDriverList() const { return mDriverList; }

		protected:
			friend class D3D11RenderAPIFactory;

			/** @copydoc RenderAPI::initialize */
			void Initialize() override;

			/** @copydoc RenderAPI::initializeWithWindow */
			void InitializeWithWindow(const SPtr<RenderWindow>& primaryWindow);

			/** @copydoc RenderAPI::destroyCore */
			void DestroyCore() override;

			/**
			 * Returns a valid command buffer. Uses the provided buffer if not null. Otherwise returns the default command
			 * buffer.
			 */
			SPtr<D3D11CommandBuffer> GetCb(const SPtr<CommandBuffer>& buffer);

			/**
			 * Creates or retrieves a proper input layout depending on the currently set vertex shader and vertex buffer.
			 *
			 * Applies the input layout to the pipeline.
			 */
			void ApplyInputLayout();

			/**
			 * Recalculates actual viewport dimensions based on currently set viewport normalized dimensions and render target
			 * and applies them for further rendering.
			 */
			void ApplyViewport();

			/** Notifies the active render target that a rendering command was queued that will potentially change its contents. */
			void NotifyRenderTargetModified();

			/** Creates and populates a set of render system capabilities describing which functionality is available. */
			void InitCapabilites(IDXGIAdapter* adapter, RenderAPICapabilities& caps) const;

		private:
			IDXGIFactory1* mDXGIFactory = nullptr;
			D3D11Device* mDevice = nullptr;

			D3D11DriverList* mDriverList = nullptr;
			D3D11Driver* mActiveD3DDriver = nullptr;

			D3D_FEATURE_LEVEL mFeatureLevel = D3D_FEATURE_LEVEL_11_0;

			D3D11HLSLProgramFactory* mHLSLFactory = nullptr;
			D3D11InputLayoutManager* mIAManager = nullptr;

			bool mPSUAVsBound = false;
			bool mCSUAVsBound = false;

			u32 mStencilRef = 0;
			Rect2 mViewportNorm = Rect2(0.0f, 0.0f, 1.0f, 1.0f);
			D3D11_VIEWPORT mViewport;
			D3D11_RECT mScissorRect;

			SPtr<VertexDeclaration> mActiveVertexDeclaration;
			SPtr<D3D11GpuProgram> mActiveVertexShader;
			SPtr<D3D11DepthStencilState> mActiveDepthStencilState;
			SPtr<D3D11CommandBuffer> mMainCommandBuffer;

			DrawOperationType mActiveDrawOp = DOT_TRIANGLE_LIST;
		};

		/** @} */
	} // namespace ct
} // namespace bs
