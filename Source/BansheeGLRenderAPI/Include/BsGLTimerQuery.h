//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsTimerQuery.h"

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
		GLuint mQueryStartObj;
		GLuint mQueryEndObj;
		bool mFinalized;
		bool mEndIssued;

		float mTimeDelta;
	};

	/** @} */
}}