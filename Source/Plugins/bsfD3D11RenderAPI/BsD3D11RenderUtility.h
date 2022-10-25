//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "Utility/BsModule.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup D3D11
		 *  @{
		 */

		/**	Helper class for DX11 rendering. */
		class D3D11RenderUtility : public Module<D3D11RenderUtility>
		{
		public:
			D3D11RenderUtility(D3D11Device* device);
			~D3D11RenderUtility();

			/**
			 * Draws a quad that clears the current viewport. This is supposed to emulate functionality available with other
			 * APIs like DX9 and OpenGL where you can clear only a part of the render target. (DX11 API only provides a way to
			 * clear the entire render target).
			 *
			 * @param[in]	clearBuffers	Combination of one or more elements of FrameBufferType denoting which buffers are
			 *								to be cleared.
			 * @param[in]	color			(optional) The color to clear the color buffer with, if enabled.
			 * @param[in]	depth			(optional) The value to initialize the depth buffer with, if enabled.
			 * @param[in]	stencil			(optional) The value to initialize the stencil buffer with, if enabled.
			 */
			void DrawClearQuad(u32 clearBuffers, const Color& color, float depth, u16 stencil);

		protected:
			/**	Initializes resources needed for drawing the clear quad. Should be called one time at start-up. */
			void InitClearQuadResources();

			D3D11Device* mDevice;

			ID3D11Buffer* mClearQuadIB;
			ID3D11Buffer* mClearQuadVB;
			ID3D11InputLayout* mClearQuadIL;
			ID3D11VertexShader* mClearQuadVS;
			ID3D11PixelShader* mClearQuadPS;

			SPtr<BlendState> mClearQuadBlendStateYesC;
			SPtr<BlendState> mClearQuadBlendStateNoC;
			SPtr<RasterizerState> mClearQuadRasterizerState;
			SPtr<DepthStencilState> mClearQuadDSStateNoD_NoS;
			SPtr<DepthStencilState> mClearQuadDSStateYesD_NoS;
			SPtr<DepthStencilState> mClearQuadDSStateYesD_YesS;
			SPtr<DepthStencilState> mClearQuadDSStateNoD_YesS;
		};

		/** @} */
	} // namespace ct
} // namespace bs
