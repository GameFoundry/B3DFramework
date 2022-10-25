//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsEventQuery.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/** @copydoc EventQuery */
		class GLEventQuery : public EventQuery
		{
		public:
			GLEventQuery(u32 deviceIdx);
			~GLEventQuery();

			/** @copydoc EventQuery::begin */
			void Begin(const SPtr<CommandBuffer>& cb = nullptr);

			/** @copydoc EventQuery::isReady */
			bool IsReady() const;

		private:
			GLuint mQueryObj = 0;
		};

		/** @} */
	} // namespace ct
} // namespace bs
