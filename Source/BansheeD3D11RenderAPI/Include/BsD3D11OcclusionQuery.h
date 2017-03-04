//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsOcclusionQuery.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/** @copydoc OcclusionQuery */
	class D3D11OcclusionQuery : public OcclusionQuery
	{
	public:
		D3D11OcclusionQuery(bool binary, UINT32 deviceIdx);
		~D3D11OcclusionQuery();

		/** @copydoc OcclusionQuery::begin */
		void begin(const SPtr<CommandBuffer>& cb = nullptr) override;

		/** @copydoc OcclusionQuery::end */
		void end(const SPtr<CommandBuffer>& cb = nullptr) override;

		/** @copydoc OcclusionQuery::isReady */
		bool isReady() const override;

		/** @copydoc OcclusionQuery::getNumSamples */
		UINT32 getNumSamples() override;

	private:
		friend class QueryManager;

		/** Resolves query results after it is ready. */
		void finalize();

		ID3D11Query* mQuery;
		ID3D11DeviceContext* mContext;
		bool mFinalized;
		bool mQueryEndCalled;

		UINT32 mNumSamples;
	};

	/** @} */
}}