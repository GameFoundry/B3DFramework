//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsTimerQuery.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of a timer query. */
	class GLTimerQuery : public TimerQuery
	{
	public:
		GLTimerQuery(UINT32 deviceIdx);
		~GLTimerQuery();

		/** @copydoc TimerQuery::begin */
		void begin(const SPtr<CommandBuffer>& cb = nullptr) override;

		/** @copydoc TimerQuery::end */
		void end(const SPtr<CommandBuffer>& cb = nullptr) override;

		/** @copydoc TimerQuery::isReady */
		bool isReady() const override;

		/** @copydoc TimerQuery::getTimeMs */
		float getTimeMs() override;

	private:
		friend class QueryManager;

		/** Processes query results and saves them for later use. To be called when query has completed. */
		void finalize();

	private:
		GLuint mQueryStartObj = 0;
		GLuint mQueryEndObj = 0;
		bool mFinalized = false;
		bool mEndIssued = false;

		float mTimeDelta = 0.0f;
	};

	/** @} */
}}