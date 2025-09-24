//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "Managers/B3DRenderStateManager.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup NullRenderAPI
		 *  @{
		 */

		/**	Handles creation of null pipeline states. */
		class NullRenderStateManager : public RenderStateManager
		{
		protected:
			SPtr<SamplerState> CreateSamplerStateInternalInternal(const SamplerStateInformation& desc, GpuDeviceFlags deviceMask) const override;
			SPtr<BlendState> CreateBlendStateInternalInternal(const BlendStateInformation& desc, u32 id) const override;
			SPtr<RasterizerState> CreateRasterizerStateInternalInternal(const RASTERIZER_STATE_DESC& desc, u32 id) const override;
			SPtr<DepthStencilState> CreateDepthStencilStateInternalInternal(const DEPTH_STENCIL_STATE_DESC& desc, u32 id) const override;
		};

		/** @copydoc BlendState */
		class NullBlendState : public BlendState
		{
		public:
			NullBlendState(const BlendStateInformation& desc, u32 id)
				: BlendState(desc, id)
			{}
		};

		/** @copydoc DepthStencilState */
		class NullDepthStencilState : public DepthStencilState
		{
		public:
			NullDepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc, u32 id)
				: DepthStencilState(desc, id)
			{}
		};

		/** @copydoc RasterizerState */
		class NullRasterizerState : public RasterizerState
		{
		public:
			NullRasterizerState(const RASTERIZER_STATE_DESC& desc, u32 id)
				: RasterizerState(desc, id)
			{}
		};

		/** @copydoc SamplerState */
		class NullSamplerState : public SamplerState
		{
		public:
			NullSamplerState(const SamplerStateInformation& desc, GpuDeviceFlags deviceMask)
				: SamplerState(desc, deviceMask)
			{}
		};

		/** @} */
	} // namespace render
} // namespace b3d
