//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsOcclusionQuery.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**	OpenGL implementation of an occlusion query. */
		class GLOcclusionQuery : public OcclusionQuery
		{
		public:
			GLOcclusionQuery(bool binary, u32 deviceIdx);
			~GLOcclusionQuery();

			void Begin(const SPtr<CommandBuffer>& cb = nullptr) override;
			void End(const SPtr<CommandBuffer>& cb = nullptr) override;
			bool IsReady() const override;
			u32 GetSampleCount() override;

		private:
			friend class QueryManager;

			/** Processes query results and saves them for later use. To be called when query has completed. */
			void Finalize();

		private:
			GLuint mQueryObj = 0;
			bool mFinalized = false;
			bool mEndIssued = false;

			u32 mNumSamples = 0;
		};

		/** @} */
	} // namespace ct
} // namespace bs
