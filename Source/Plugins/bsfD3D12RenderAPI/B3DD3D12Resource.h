//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12RenderAPI
		 *  @{
		 */

		/** Base class for all D3D12 GPU resources that need to be tracked for synchronization purposes. */
		class D3D12Resource
		{
		public:
			D3D12Resource();
			virtual ~D3D12Resource();

			/** Returns the D3D12 resource. */
			virtual ID3D12Resource* GetD3D12Resource() const = 0;

			/** Returns the current resource state. */
			D3D12_RESOURCE_STATES GetCurrentState() const { return mCurrentState; }

			/** Sets the current resource state. */
			void SetCurrentState(D3D12_RESOURCE_STATES state) { mCurrentState = state; }

		protected:
			D3D12_RESOURCE_STATES mCurrentState = D3D12_RESOURCE_STATE_COMMON;
		};

		/** @} */
	} // namespace render
} // namespace b3d
