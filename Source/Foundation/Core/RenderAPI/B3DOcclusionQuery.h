//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DCorePrerequisites.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup RenderAPI
		 *  @{
		 */

		/** Represents a query that counts number of samples (pixels) rendered by the GPU while the query is active. */
		class B3D_EXPORT OcclusionQuery
		{
		public:
			OcclusionQuery(bool binary)
				:mBinary(binary)
			{ }
			virtual ~OcclusionQuery() = default;

			/**
			 * Schedules the query start in the command buffer. Any draw calls scheduled in the same command buffer after this call will
			 * have any rendered samples (pixels) counted in the query. Must be followed by a call to End(). You are
			 * not allowed to call this again until the query completes, at which point it can be re-scheduled.
			 */
			virtual void Begin(GpuCommandBuffer& commandBuffer) = 0;

			/**
			 * Schedules the query end in the command buffer. Any commands past this call will not be measured
			 * by the query. Must be preceded by Begin().
			 */
			virtual void End(GpuCommandBuffer& commandBuffer) = 0;

			/** Check if GPU has processed the query. */
			virtual bool IsReady() const = 0;

			/**
			 * Returns the number of samples (pixels) that passed the depth and stencil test between query start and end.
			 * If the query is binary, this will return 0 or 1. 1 meaning one or more samples were rendered, but will not
			 * give you the exact count. Only valid when IsReady() returns true.
			 */
			virtual u32 GetSampleCount() = 0;

		protected:
			bool mBinary;
		};

		/** @} */
	} // namespace render
} // namespace b3d
