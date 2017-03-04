//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsSamplerState.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	DirectX 11 implementation of a sampler state. Wraps a DX11 sampler state object. */
	class D3D11SamplerState : public SamplerState
	{
	public:
		~D3D11SamplerState();
		ID3D11SamplerState* getInternal() const { return mSamplerState; }

	protected:
		friend class D3D11RenderStateManager;

		D3D11SamplerState(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc SamplerState::createInternal */
		void createInternal() override;

		ID3D11SamplerState* mSamplerState;
	};

	/** @} */
}}