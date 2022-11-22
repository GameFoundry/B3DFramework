//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsOcclusionQuery.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup D3D11
		 *  @{
		 */

		/** @copydoc OcclusionQuery */
		class D3D11OcclusionQuery : public OcclusionQuery
		{
		public:
			D3D11OcclusionQuery(bool binary, u32 deviceIdx);
			~D3D11OcclusionQuery();

			void Begin(const SPtr<CommandBuffer>& cb = nullptr) override;
			void End(const SPtr<CommandBuffer>& cb = nullptr) override;
			bool IsReady() const override;
			u32 GetSampleCount() override;

		private:
			friend class QueryManager;

			/** Resolves query results after it is ready. */
			void Finalize();

			ID3D11Query* mQuery = nullptr;
			ID3D11DeviceContext* mContext = nullptr;
			bool mFinalized = false;
			bool mQueryEndCalled = false;

			u32 mNumSamples = 0;
		};

		/** @} */
	} // namespace ct
} // namespace bs
