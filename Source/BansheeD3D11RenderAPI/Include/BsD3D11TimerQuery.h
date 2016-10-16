//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsTimerQuery.h"

namespace BansheeEngine
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/** @copydoc TimerQuery */
	class BS_D3D11_EXPORT D3D11TimerQuery : public TimerQuery
	{
	public:
		D3D11TimerQuery(UINT32 deviceIdx);
		~D3D11TimerQuery();

		/** @copydoc TimerQuery::begin */
		void begin() override;

		/** @copydoc TimerQuery::end */
		void end() override;

		/** @copydoc TimerQuery::isReady */
		bool isReady() const override;

		/** @copydoc TimerQuery::getTimeMs */
		float getTimeMs() override;

	private:
		/**	Resolve timing information after the query has finished. */
		void finalize();

		bool mFinalized;
		bool mQueryEndCalled;
		float mTimeDelta;

		ID3D11Query* mBeginQuery;
		ID3D11Query* mEndQuery;
		ID3D11Query* mDisjointQuery;
		ID3D11DeviceContext* mContext;
	};

	/** @} */
}