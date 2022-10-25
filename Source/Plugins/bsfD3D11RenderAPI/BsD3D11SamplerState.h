//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsSamplerState.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup D3D11
		 *  @{
		 */

		/**	DirectX 11 implementation of a sampler state. Wraps a DX11 sampler state object. */
		class D3D11SamplerState : public SamplerState
		{
		public:
			~D3D11SamplerState();

			ID3D11SamplerState* GetInternal() const { return mSamplerState; }

		protected:
			friend class D3D11RenderStateManager;

			D3D11SamplerState(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask);

			/** @copydoc SamplerState::createInternal */
			void CreateInternal();

			ID3D11SamplerState* mSamplerState = nullptr;
		};

		/** @} */
	} // namespace ct
} // namespace bs
