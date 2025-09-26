//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DCorePrerequisites.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup RenderAPI-Internal
		 *  @{
		 */

		/**
		 * Represents a GPU query that measures execution time of GPU operations. The query will measure any GPU operations
		 * that take place between its Begin() and End() calls.
		 */
		class B3D_EXPORT TimerQuery
		{
		public:
			virtual ~TimerQuery() = default;

			/**
			 * Schedules the query start in the command buffer. The query will measure the time it takes to execute
			 * any commands queued after it on the same command buffer. Must be followed by an End() call. You are
			 * not allowed to call this again until the query completes, at which point it can be re-scheduled.
			 */
			virtual void Begin(GpuCommandBuffer& commandBuffer) = 0;

			/**
			 * Schedules the query end in the command buffer. Any commands past this call will not be measured
			 * by the query. Must be preceded by Begin().
			 */
			virtual void End(GpuCommandBuffer& commandBuffer) = 0;

			/**	Checks if query results are ready. */
			virtual bool IsReady() const = 0;

			/** Returns the time it took for the query to execute, in milliseconds. Only valid after IsReady() returns true. */
			virtual float GetTimeMs() = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d
