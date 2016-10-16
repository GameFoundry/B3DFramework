//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsEventQuery.h"

namespace BansheeEngine
{
	/** @addtogroup GL
	 *  @{
	 */

	/** @copydoc EventQuery */
	class BS_RSGL_EXPORT GLEventQuery : public EventQuery
	{
	public:
		GLEventQuery(UINT32 deviceIdx);
		~GLEventQuery();

		/** @copydoc EventQuery::begin */
		void begin() override;

		/** @copydoc EventQuery::isReady */
		bool isReady() const override;

	private:
		GLuint mQueryObj;
	};

	/** @} */
}