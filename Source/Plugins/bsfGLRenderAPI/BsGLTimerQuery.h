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

			void Begin(const SPtr<CommandBuffer>& cb = nullptr) override;
			void End(const SPtr<CommandBuffer>& cb = nullptr) override;
			bool IsReady() const override;
			float GetTimeMs() override;

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
