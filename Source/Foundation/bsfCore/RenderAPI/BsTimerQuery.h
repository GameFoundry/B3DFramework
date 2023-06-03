//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup RenderAPI-Internal
		 *  @{
		 */

		/** Represents a GPU query that records a timestamp on the GPU that may be used for measuring execution time. */
		class B3D_CORE_EXPORT TimerQuery
		{
		public:
			virtual ~TimerQuery() = default;

			/** Schedules the query in the command buffer. You are not allowed to call this again until the query completes, at which point it can be re-scheduled. */
			virtual void Begin(GpuCommandBuffer& commandBuffer) = 0;

			/**	Checks if query results are ready. */
			virtual bool IsReady() const = 0;

			/**
			 * Returns the timestamp recorded by the query. This time is in GPU-specific timestamp units and can be converted
			 * to milliseconds by calling ConvertTimestampUnitsToMilliseconds. For best precision it's preferable to
			 * convert the difference between two timestamps, rather than converting the timestamps first and then computing the difference.
			 *
			 * Only valid after IsReady() returns true.
			 */
			virtual u64 GetTimestamp() = 0;

			/** Converts a GPU timestamp value into milliseconds. */
			virtual double ConvertTimestampUnitsToMilliseconds(u64 timestamp) = 0;
		};

		/** @} */
	} // namespace ct
} // namespace bs
