//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup GpuBackend
		 *  @{
		 */

		/** Represents a GPU query that gets signaled when GPU starts processing the query. */
		class B3D_EXPORT EventQuery
		{
		public:
			EventQuery();
			virtual ~EventQuery();

			/** Schedules the query for execution on the command buffer. Once the GPU reaches this point the query will be set in the signaled state. */
			virtual void Begin(GpuCommandBuffer& commandBuffer) = 0;

			/**	Checks if query results are ready. */
			virtual bool IsReady() const = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d
