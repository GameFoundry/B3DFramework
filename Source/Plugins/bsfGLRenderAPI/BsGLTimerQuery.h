//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsTimerQuery.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**	OpenGL implementation of a timer query. */
		class GLTimerQuery : public TimerQuery
		{
		public:
			GLTimerQuery(u32 deviceIdx);
			~GLTimerQuery();

			/** @copydoc TimerQuery::begin */
			void Begin(const SPtr<CommandBuffer>& cb = nullptr);

			/** @copydoc TimerQuery::end */
			void End(const SPtr<CommandBuffer>& cb = nullptr);

			/** @copydoc TimerQuery::isReady */
			bool IsReady() const;

			/** @copydoc TimerQuery::getTimeMs */
			float GetTimeMs();

		private:
			friend class QueryManager;

			/** Processes query results and saves them for later use. To be called when query has completed. */
			void Finalize();

		private:
			GLuint mQueryStartObj = 0;
			GLuint mQueryEndObj = 0;
			bool mFinalized = false;
			bool mEndIssued = false;

			float mTimeDelta = 0.0f;
		};

		/** @} */
	} // namespace ct
} // namespace bs
