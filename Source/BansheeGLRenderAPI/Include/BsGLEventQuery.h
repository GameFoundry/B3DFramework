//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsEventQuery.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/** @copydoc EventQuery */
	class GLEventQuery : public EventQuery
	{
	public:
		GLEventQuery(UINT32 deviceIdx);
		~GLEventQuery();

		/** @copydoc EventQuery::begin */
		void begin(const SPtr<CommandBuffer>& cb = nullptr) override;

		/** @copydoc EventQuery::isReady */
		bool isReady() const override;

	private:
		GLuint mQueryObj;
	};

	/** @} */
}}